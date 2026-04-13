#!/usr/bin/env python3
"""
Async mock backend for iot_terminal_ui.
Requires: pip install aiohttp
"""
import argparse
import json
from typing import Dict
from aiohttp import web

# ── shared state ──────────────────────────────────────────────────────────────
balances: Dict[str, int] = {}
spin_counter = 0

PAYLINES = (
    (0, 0, 0, 0, 0), (1, 1, 1, 1, 1), (2, 2, 2, 2, 2),
    (0, 1, 2, 1, 0), (2, 1, 0, 1, 2), (0, 0, 1, 0, 0),
    (2, 2, 1, 2, 2), (1, 0, 0, 0, 1), (1, 2, 2, 2, 1),
    (0, 1, 0, 1, 0), (2, 1, 2, 1, 2), (2, 2, 1, 0, 0),
    (0, 0, 1, 2, 2), (0, 1, 2, 2, 2), (2, 1, 0, 0, 0),
    (0, 1, 1, 1, 0), (2, 1, 1, 1, 2), (0, 1, 2, 1, 2),
    (2, 1, 0, 1, 0), (1, 0, 1, 2, 1),
)


# ── helpers ───────────────────────────────────────────────────────────────────
def device_balance(device_id: str) -> int:
    if device_id not in balances:
        balances[device_id] = 1000
    return balances[device_id]


def payout_multiplier(symbols, reel_count: int, row_count: int) -> int:
    if reel_count == 3 or row_count < 3:
        return 10 if symbols[0][0] == symbols[0][1] == symbols[0][2] else 0

    multiplier = 0
    for line in PAYLINES:
        anchor = symbols[line[0]][0]
        matched = 1
        for reel in range(1, reel_count):
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


def apply_spin_balance(device_id: str, bet: int, symbols, reel_count: int, row_count: int):
    mult = payout_multiplier(symbols, reel_count, row_count)
    balances[device_id] = max(0, device_balance(device_id) - bet + bet * mult)
    return mult, balances[device_id]


def json_resp(status: int, payload: dict) -> web.Response:
    return web.Response(
        status=status,
        content_type="application/json",
        text=json.dumps(payload, separators=(",", ":")) + "\n",
    )


async def read_json(request: web.Request) -> dict:
    try:
        return await request.json()
    except Exception:
        return {}


# ── route handlers ────────────────────────────────────────────────────────────
async def health(_request: web.Request) -> web.Response:
    return json_resp(200, {"status": "ok", "message": "mock backend healthy"})


async def auth(request: web.Request) -> web.Response:
    payload = await read_json(request)
    device_id = payload.get("deviceId") or "unknown"
    device_balance(device_id)
    return json_resp(200, {
        "status": "ok",
        "authorized": True,
        "token": f"mock-session-token-{device_id}",
        "userName": f"Player {device_id[-4:]}",
        "message": "mock auth ok",
    })


async def balance(request: web.Request) -> web.Response:
    device_id = request.match_info.get("device_id", "unknown")
    return json_resp(200, {
        "status": "ok",
        "balance": device_balance(device_id),
        "currency": "USD",
        "message": "mock balance",
    })


async def spin_3(request: web.Request) -> web.Response:
    global spin_counter
    payload = await read_json(request)
    device_id = payload.get("deviceId") or "unknown"

    if "clientBalance" in payload:
        balances[device_id] = max(0, int(payload["clientBalance"]))

    bet = int(payload.get("bet") or 0)
    if device_balance(device_id) < bet:
        return json_resp(409, {
            "status": "error",
            "error": "insufficient_balance",
            "message": "mock balance is too low",
        })

    spin_counter += 1
    c = spin_counter
    symbols = [[c % 6, (c + 1) % 6, (c + 2) % 6]]
    mult, bal = apply_spin_balance(device_id, bet, symbols, 3, 1)

    return json_resp(200, {
        "status": "ok",
        "mode": "3-reels",
        "symbols": symbols,
        "payoutMultiplier": mult,
        "balance": bal,
        "message": "mock 3 reels",
    })


async def spin_5(request: web.Request) -> web.Response:
    global spin_counter
    payload = await read_json(request)
    device_id = payload.get("deviceId") or "unknown"

    if "clientBalance" in payload:
        balances[device_id] = max(0, int(payload["clientBalance"]))

    bet = int(payload.get("bet") or 0)
    if device_balance(device_id) < bet:
        return json_resp(409, {
            "status": "error",
            "error": "insufficient_balance",
            "message": "mock balance is too low",
        })

    spin_counter += 1
    offset = spin_counter % 6
    symbols = [
        [(offset + i) % 6 for i in range(5)],
        [(offset + i + 1) % 6 for i in range(5)],
        [(offset + i + 2) % 6 for i in range(5)],
    ]
    mult, bal = apply_spin_balance(device_id, bet, symbols, 5, 3)

    return json_resp(200, {
        "status": "ok",
        "mode": "5-reels",
        "symbols": symbols,
        "payoutMultiplier": mult,
        "balance": bal,
        "message": "mock 5 reels",
    })


async def events(request: web.Request) -> web.Response:
    payload = await read_json(request)
    device_id = payload.get("deviceId") or "unknown"
    event_type = payload.get("eventType") or ""
    if event_type in ("spin_win", "spin_loss") and "value" in payload:
        balances[device_id] = max(0, int(payload["value"]))
    return json_resp(202, {"status": "ok", "message": "mock event accepted"})


async def default(_request: web.Request) -> web.Response:
    return json_resp(200, {"status": "ok", "message": "mock default response"})


# ── app factory ───────────────────────────────────────────────────────────────
def make_app() -> web.Application:
    app = web.Application()
    app.router.add_get( "/api/v1/health",                          health)
    app.router.add_post("/api/v1/devices/auth",                    auth)
    app.router.add_get( "/api/v1/devices/{device_id}/balance",     balance)
    app.router.add_post("/api/v1/games/slot/3-reels/spin",         spin_3)
    app.router.add_post("/api/v1/games/slot/5-reels/spin",         spin_5)
    app.router.add_post("/api/v1/devices/events",                  events)
    app.router.add_route("*", "/{tail:.*}",                        default)
    return app


def main():
    parser = argparse.ArgumentParser(description="Async mock backend for iot_terminal_ui.")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    app = make_app()
    print(f"Mock server listening on http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    web.run_app(app, host=args.host, port=args.port, access_log=None)


if __name__ == "__main__":
    main()
