#!/usr/bin/env python3
"""
Mock backend for iot_terminal_ui.
Requires: pip install aiohttp
"""
import argparse
import json
import os
import random
import secrets
import time
from dataclasses import dataclass, field
from typing import Any, Dict, List, Tuple

from aiohttp import web

API_PREFIX = "/api/v1"
DEFAULT_CURRENCY = "USD"
SYMBOL_MODULO = 6

PAYLINES = (
    (0, 0, 0, 0, 0),
    (1, 1, 1, 1, 1),
    (2, 2, 2, 2, 2),
    (0, 1, 2, 1, 0),
    (2, 1, 0, 1, 2),
    (0, 0, 1, 0, 0),
    (2, 2, 1, 2, 2),
    (1, 0, 0, 0, 1),
    (1, 2, 2, 2, 1),
    (0, 1, 0, 1, 0),
    (2, 1, 2, 1, 2),
    (2, 2, 1, 0, 0),
    (0, 0, 1, 2, 2),
    (0, 1, 2, 2, 2),
    (2, 1, 0, 0, 0),
    (0, 1, 1, 1, 0),
    (2, 1, 1, 1, 2),
    (0, 1, 2, 1, 2),
    (2, 1, 0, 1, 0),
    (1, 0, 1, 2, 1),
)


@dataclass
class DeviceSession:
    token: str
    account_id: str
    firmware_version: str
    capabilities: List[str] = field(default_factory=list)
    user_name: str = "Player"
    spin_counter: int = 0
    initial_balance: int = 1000
    loss_streak: int = 0
    win_streak: int = 0
    total_bet: int = 0
    total_payout: int = 0
    last_spin_at: int = 0

device_accounts: Dict[str, List[Dict[str, Any]]] = {}
balances: Dict[str, int] = {}
sessions: Dict[str, DeviceSession] = {}


def int_env(name: str, default: int) -> int:
    value = os.getenv(name)
    if value is None:
        return default
    try:
        return int(value)
    except ValueError:
        return default


def float_env(name: str, default: float) -> float:
    value = os.getenv(name)
    if value is None:
        return default
    try:
        return float(value)
    except ValueError:
        return default


INITIAL_BALANCE = max(1, int_env("INITIAL_BALANCE", 1000))
WIN_BIAS = max(0.0, float_env("WIN_BIAS", 1.0))
BASE_WIN_CHANCE_3 = max(0.0, float_env("BASE_WIN_CHANCE_3", 0.08))
BASE_WIN_CHANCE_5 = max(0.0, float_env("BASE_WIN_CHANCE_5", 0.18))
LOSS_STREAK_STEP = max(0.0, float_env("LOSS_STREAK_STEP", 0.025))
LOW_BALANCE_THRESHOLD = min(1.0, max(0.0, float_env("LOW_BALANCE_THRESHOLD", 0.5)))
LOW_BALANCE_BOOST = max(0.0, float_env("LOW_BALANCE_BOOST", 0.10))
MAX_WIN_CHANCE_3 = min(1.0, max(0.0, float_env("MAX_WIN_CHANCE_3", 0.35)))
MAX_WIN_CHANCE_5 = min(1.0, max(0.0, float_env("MAX_WIN_CHANCE_5", 0.45)))


def json_resp(status_code: int, payload: Dict[str, Any]) -> web.Response:
    return web.Response(
        status=status_code,
        content_type="application/json",
        text=json.dumps(payload, separators=(",", ":"), ensure_ascii=True) + "\n",
    )


def error_resp(status_code: int, error: str, message: str) -> web.Response:
    return json_resp(
        status_code,
        {
            "status": "error",
            "error": error,
            "message": message,
        },
    )


@web.middleware
async def cors_middleware(request: web.Request, handler):
    if request.method == "OPTIONS":
        response = web.Response(status=204)
    else:
        response = await handler(request)

    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Methods"] = "GET,POST,OPTIONS"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type,Authorization,Accept"
    response.headers["Access-Control-Max-Age"] = "86400"
    return response


def balance_key(device_id: str, account_id: str) -> str:
    return f"{device_id}::{account_id}"


def make_stub_accounts(device_id: str) -> List[Dict[str, Any]]:
    suffix = device_id[-4:].upper() if len(device_id) >= 4 else "0000"
    return [
        {"accountId": f"{suffix}-A", "displayName": "Alex", "balance": INITIAL_BALANCE},
        {"accountId": f"{suffix}-M", "displayName": "Mira", "balance": INITIAL_BALANCE + 250},
        {"accountId": f"{suffix}-S", "displayName": "Sam", "balance": max(100, INITIAL_BALANCE - 300)},
    ]


def ensure_accounts(device_id: str) -> List[Dict[str, Any]]:
    if device_id not in device_accounts:
        accounts = make_stub_accounts(device_id)
        device_accounts[device_id] = accounts
        for account in accounts:
            balances[balance_key(device_id, account["accountId"])] = int(account["balance"])
    return device_accounts[device_id]


def find_account(device_id: str, account_id: str) -> Dict[str, Any] | None:
    for account in ensure_accounts(device_id):
        if account["accountId"] == account_id:
            return account
    return None


def default_account_id(device_id: str) -> str:
    accounts = ensure_accounts(device_id)
    return accounts[0]["accountId"] if accounts else "default"


def ensure_balance(device_id: str, account_id: str | None = None) -> int:
    if not account_id:
        account_id = default_account_id(device_id)
    key = balance_key(device_id, account_id)
    if key not in balances:
        balances[key] = INITIAL_BALANCE
    return balances[key]


async def read_json(request: web.Request) -> Tuple[Dict[str, Any], web.Response | None]:
    try:
        payload = await request.json()
    except Exception:
        return {}, error_resp(400, "invalid_json", "Request body must be valid JSON")
    if not isinstance(payload, dict):
        return {}, error_resp(400, "invalid_payload", "JSON payload must be an object")
    return payload, None


def extract_non_empty_str(payload: Dict[str, Any], field_name: str) -> Tuple[str, str | None]:
    value = payload.get(field_name)
    if not isinstance(value, str) or not value.strip():
        return "", f"Field '{field_name}' must be a non-empty string"
    return value.strip(), None


def validate_bet(payload: Dict[str, Any]) -> Tuple[int, str | None]:
    bet = payload.get("bet")
    if not isinstance(bet, int) or bet <= 0:
        return 0, "Field 'bet' must be a positive integer"
    return bet, None


def payout_multiplier(symbols: List[List[int]], mode: str) -> int:
    if mode == "3-reels":
        return 10 if symbols[0][0] == symbols[0][1] == symbols[0][2] else 0

    multiplier = 0
    for line in PAYLINES:
        anchor = symbols[line[0]][0]
        matched = 1
        for reel in range(1, 5):
            if symbols[line[reel]][reel] != anchor:
                break
            matched += 1
        if matched >= 5:
            multiplier += 10
        elif matched == 4:
            multiplier += 5
        elif matched == 3:
            multiplier += 2
    return multiplier


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(value, high))


def random_symbol() -> int:
    return random.randrange(SYMBOL_MODULO)


def random_grid(rows: int, reels: int) -> List[List[int]]:
    return [[random_symbol() for _ in range(reels)] for _ in range(rows)]


def compute_win_chance(session: DeviceSession, balance: int, mode: str) -> float:
    base = BASE_WIN_CHANCE_3 if mode == "3-reels" else BASE_WIN_CHANCE_5
    cap = MAX_WIN_CHANCE_3 if mode == "3-reels" else MAX_WIN_CHANCE_5
    balance_ratio = balance / max(1, session.initial_balance)

    chance = base + session.loss_streak * LOSS_STREAK_STEP
    if balance_ratio < LOW_BALANCE_THRESHOLD:
        chance += LOW_BALANCE_BOOST
    chance *= WIN_BIAS
    return clamp(chance, 0.0, cap)


def generate_3_reels(should_win: bool) -> List[List[int]]:
    if should_win:
        symbol = random_symbol()
        return [[symbol, symbol, symbol]]

    row = [random_symbol(), random_symbol(), random_symbol()]
    if row[0] == row[1] == row[2]:
        row[2] = (row[2] + 1) % SYMBOL_MODULO
    return [row]


def break_any_3plus_chains_5(symbols: List[List[int]]) -> List[List[int]]:
    # Ensure no payline starts with 3 identical symbols from reel 1.
    for _ in range(20):
        changed = False
        for line in PAYLINES:
            s0 = symbols[line[0]][0]
            s1 = symbols[line[1]][1]
            s2 = symbols[line[2]][2]
            if s0 == s1 == s2:
                symbols[line[2]][2] = (s2 + 1) % SYMBOL_MODULO
                changed = True
        if not changed:
            break
    return symbols


def generate_5_reels(should_win: bool) -> List[List[int]]:
    if not should_win:
        for _ in range(80):
            symbols = random_grid(rows=3, reels=5)
            if payout_multiplier(symbols, mode="5-reels") == 0:
                return symbols
        symbols = random_grid(rows=3, reels=5)
        return break_any_3plus_chains_5(symbols)

    symbols = random_grid(rows=3, reels=5)
    line = random.choice(PAYLINES)
    symbol = random_symbol()
    roll = random.random()
    if roll < 0.60:
        win_len = 3
    elif roll < 0.90:
        win_len = 4
    else:
        win_len = 5

    for reel in range(win_len):
        row = line[reel]
        symbols[row][reel] = symbol
    return symbols


def bearer_token_from_header(request: web.Request) -> str:
    auth_header = request.headers.get("Authorization", "").strip()
    if not auth_header.lower().startswith("bearer "):
        return ""
    token = auth_header[7:].strip()
    return token


def verify_session(payload: Dict[str, Any]) -> Tuple[str, DeviceSession, web.Response | None]:
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return "", DeviceSession(token="", account_id="", firmware_version=""), error_resp(400, "validation_error", err)

    token = payload.get("sessionToken")
    if token is None:
        return "", DeviceSession(token="", account_id="", firmware_version=""), error_resp(401, "unauthorized", "Missing or invalid session token")
    if not isinstance(token, str) or not token.strip():
        return "", DeviceSession(token="", account_id="", firmware_version=""), error_resp(401, "unauthorized", "Missing or invalid session token")
    token = token.strip()

    session = sessions.get(device_id)
    if session is None or session.token != token:
        return "", DeviceSession(token="", account_id="", firmware_version=""), error_resp(401, "unauthorized", "Session is not valid for this device")

    return device_id, session, None


async def health(_request: web.Request) -> web.Response:
    return json_resp(200, {"status": "ok", "message": "backend is healthy"})


async def auth(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp

    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    account_id = payload.get("accountId")
    if account_id is None:
        account_id = default_account_id(device_id)
    if not isinstance(account_id, str) or not account_id.strip():
        return error_resp(400, "validation_error", "Field 'accountId' must be a non-empty string")
    account_id = account_id.strip()
    account = find_account(device_id, account_id)
    if account is None:
        return error_resp(404, "account_not_found", "Requested account is not available for this device")

    firmware_version, err = extract_non_empty_str(payload, "firmwareVersion")
    if err:
        return error_resp(400, "validation_error", err)

    capabilities = payload.get("capabilities", [])
    if not isinstance(capabilities, list) or any(not isinstance(c, str) for c in capabilities):
        return error_resp(400, "validation_error", "Field 'capabilities' must be an array of strings")

    ensure_balance(device_id, account_id)
    token = secrets.token_urlsafe(24)
    user_name = account["displayName"]
    existing = sessions.get(device_id)
    if existing:
        existing.token = token
        existing.account_id = account_id
        existing.firmware_version = firmware_version
        existing.capabilities = capabilities
        existing.user_name = user_name
    else:
        sessions[device_id] = DeviceSession(
            token=token,
            account_id=account_id,
            firmware_version=firmware_version,
            capabilities=capabilities,
            user_name=user_name,
            initial_balance=ensure_balance(device_id, account_id),
        )

    return json_resp(
        200,
        {
            "status": "ok",
            "authorized": True,
            "accountId": account_id,
            "token": token,
            "userName": user_name,
            "message": "auth ok",
        },
    )


async def list_accounts(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")

    accounts = []
    for account in ensure_accounts(device_id):
        account_id = account["accountId"]
        accounts.append(
            {
                "accountId": account_id,
                "displayName": account["displayName"],
                "balance": ensure_balance(device_id, account_id),
            }
        )

    return json_resp(
        200,
        {
            "status": "ok",
            "accounts": accounts,
            "message": "accounts",
        },
    )


async def balance(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")
    session = sessions.get(device_id)
    account_id = session.account_id if session is not None else default_account_id(device_id)

    return json_resp(
        200,
        {
            "status": "ok",
            "balance": ensure_balance(device_id, account_id),
            "currency": DEFAULT_CURRENCY,
            "message": "balance",
        },
    )


async def run_spin(request: web.Request, mode: str, rows: int, reels: int) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp

    if "sessionToken" not in payload:
        header_token = bearer_token_from_header(request)
        if header_token:
            payload["sessionToken"] = header_token

    device_id, session, err_resp = verify_session(payload)
    if err_resp:
        return err_resp

    payload_mode = payload.get("mode")
    if payload_mode != mode:
        return error_resp(400, "validation_error", f"Field 'mode' must be '{mode}'")

    bet, err = validate_bet(payload)
    if err:
        return error_resp(400, "validation_error", err)

    if "clientBalance" in payload:
        client_balance = payload.get("clientBalance")
        if not isinstance(client_balance, int) or client_balance < 0:
            return error_resp(400, "validation_error", "Field 'clientBalance' must be a non-negative integer")

    current_balance = ensure_balance(device_id, session.account_id)
    if current_balance < bet:
        return error_resp(409, "insufficient_balance", "balance is too low")

    session.spin_counter += 1
    win_chance = compute_win_chance(session, current_balance, mode)
    should_win = random.random() < win_chance
    symbols = generate_3_reels(should_win) if mode == "3-reels" else generate_5_reels(should_win)

    multiplier = payout_multiplier(symbols, mode=mode)
    payout = bet * multiplier
    new_balance = max(0, current_balance - bet + payout)
    balances[balance_key(device_id, session.account_id)] = new_balance
    session.total_bet += bet
    session.total_payout += payout
    session.last_spin_at = int(time.time())
    if multiplier > 0:
        session.win_streak += 1
        session.loss_streak = 0
    else:
        session.loss_streak += 1
        session.win_streak = 0

    return json_resp(
        200,
        {
            "status": "ok",
            "mode": mode,
            "symbols": symbols,
            "payoutMultiplier": multiplier,
            "balance": new_balance,
            "message": "spin ok",
        },
    )


async def spin_3(request: web.Request) -> web.Response:
    return await run_spin(request, mode="3-reels", rows=1, reels=3)


async def spin_5(request: web.Request) -> web.Response:
    return await run_spin(request, mode="5-reels", rows=3, reels=5)


async def device_events(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp

    if "sessionToken" not in payload:
        header_token = bearer_token_from_header(request)
        if header_token:
            payload["sessionToken"] = header_token

    device_id, session, err_resp = verify_session(payload)
    if err_resp:
        return err_resp

    event_type, err = extract_non_empty_str(payload, "eventType")
    if err:
        return error_resp(400, "validation_error", err)

    if "value" in payload:
        value = payload.get("value")
        if not isinstance(value, int) or value < 0:
            return error_resp(400, "validation_error", "Field 'value' must be a non-negative integer")
        balances[balance_key(device_id, session.account_id)] = value

    session.spin_counter += 0
    return json_resp(202, {"status": "ok", "message": f"Event '{event_type}' accepted"})


async def not_found(_request: web.Request) -> web.Response:
    return error_resp(404, "not_found", "Route not found")


def make_app() -> web.Application:
    app = web.Application(middlewares=[cors_middleware])
    app.router.add_get(f"{API_PREFIX}/health", health)
    app.router.add_post(f"{API_PREFIX}/devices/auth", auth)
    app.router.add_get(f"{API_PREFIX}/devices/{{deviceId}}/accounts", list_accounts)
    app.router.add_get(f"{API_PREFIX}/devices/{{deviceId}}/balance", balance)
    app.router.add_post(f"{API_PREFIX}/games/slot/3-reels/spin", spin_3)
    app.router.add_post(f"{API_PREFIX}/games/slot/5-reels/spin", spin_5)
    app.router.add_post(f"{API_PREFIX}/devices/events", device_events)
    app.router.add_route("OPTIONS", "/{tail:.*}", not_found)
    app.router.add_route("*", "/{tail:.*}", not_found)
    return app


def main() -> None:
    parser = argparse.ArgumentParser(description="Mock backend for iot_terminal_ui")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    app = make_app()
    print(f"Mock server listening on http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    web.run_app(app, host=args.host, port=args.port, access_log=None)


if __name__ == "__main__":
    main()
