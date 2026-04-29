#!/usr/bin/env python3
"""Backend with SQLite storage, admin panel and account-scoped gameplay API."""
import argparse
import base64
import hashlib
import json
import os
import random
import secrets
import sqlite3
import time
import uuid
from pathlib import Path
from typing import Any, Dict, List, Tuple

from aiohttp import web

API_PREFIX = "/api/v1"
DEFAULT_CURRENCY = "USD"
SYMBOL_MODULO = 6
DB_PATH = os.getenv("BACKEND_DB_PATH", str(Path(__file__).with_name("backend.sqlite3")))
ADMIN_PANEL_PATH = Path(__file__).with_name("admin_panel.html")
USER_PANEL_PATH = Path(__file__).with_name("user_panel.html")
SESSION_TTL_SEC = max(60, int(os.getenv("SESSION_TTL_SEC", "43200")))
ADMIN_SESSION_TTL_SEC = max(300, int(os.getenv("ADMIN_SESSION_TTL_SEC", "28800")))
USER_PORTAL_SESSION_TTL_SEC = max(300, int(os.getenv("USER_PORTAL_SESSION_TTL_SEC", "21600")))
SUPERUSER_USERNAME = os.getenv("SUPERUSER_USERNAME", "admin")
SUPERUSER_PASSWORD = os.getenv("SUPERUSER_PASSWORD", "admin123")

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


DEFAULT_SETTINGS = {
    "initial_balance": max(1, int_env("INITIAL_BALANCE", 1000)),
    "win_bias": max(0.0, float_env("WIN_BIAS", 1.0)),
    "base_win_chance_3": max(0.0, float_env("BASE_WIN_CHANCE_3", 0.08)),
    "base_win_chance_5": max(0.0, float_env("BASE_WIN_CHANCE_5", 0.18)),
    "loss_streak_step": max(0.0, float_env("LOSS_STREAK_STEP", 0.025)),
    "max_win_chance_3": min(1.0, max(0.0, float_env("MAX_WIN_CHANCE_3", 0.35))),
    "max_win_chance_5": min(1.0, max(0.0, float_env("MAX_WIN_CHANCE_5", 0.45))),
}


def json_resp(status_code: int, payload: Dict[str, Any]) -> web.Response:
    return web.Response(
        status=status_code,
        content_type="application/json",
        text=json.dumps(payload, separators=(",", ":"), ensure_ascii=True) + "\n",
    )


def error_resp(status_code: int, error: str, message: str) -> web.Response:
    return json_resp(status_code, {"status": "error", "error": error, "message": message})


@web.middleware
async def cors_middleware(request: web.Request, handler):
    if request.method == "OPTIONS":
        response = web.Response(status=204)
    else:
        response = await handler(request)
    response.headers["Access-Control-Allow-Origin"] = "*"
    response.headers["Access-Control-Allow-Methods"] = "GET,POST,PUT,PATCH,DELETE,OPTIONS"
    response.headers["Access-Control-Allow-Headers"] = "Content-Type,Authorization,Accept,X-Admin-Token"
    response.headers["Access-Control-Max-Age"] = "86400"
    return response


def db_conn() -> sqlite3.Connection:
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA foreign_keys=ON")
    return conn


def hash_password(password: str, salt: str | None = None) -> str:
    salt_value = salt or secrets.token_hex(16)
    digest = hashlib.sha256((salt_value + password).encode("utf-8")).hexdigest()
    return f"{salt_value}${digest}"


def verify_password(password: str, stored_hash: str) -> bool:
    parts = stored_hash.split("$", 1)
    if len(parts) != 2:
        return False
    return hash_password(password, parts[0]) == stored_hash


def generate_account_id() -> str:
    # UUIDv4 bytes encoded to URL-safe base64 without padding: 22 chars.
    raw = uuid.uuid4().bytes
    return base64.urlsafe_b64encode(raw).decode("ascii").rstrip("=")


def init_db() -> None:
    with db_conn() as conn:
        conn.executescript(
            """
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS admins (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                is_superuser INTEGER NOT NULL DEFAULT 0,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS devices (
                device_id TEXT PRIMARY KEY,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS accounts (
                account_id TEXT PRIMARY KEY,
                user_id INTEGER,
                display_name TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL
            );
            CREATE TABLE IF NOT EXISTS device_accounts (
                device_id TEXT NOT NULL,
                account_id TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                PRIMARY KEY (device_id, account_id),
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS balances (
                device_id TEXT NOT NULL,
                account_id TEXT NOT NULL,
                balance INTEGER NOT NULL,
                updated_at INTEGER NOT NULL,
                PRIMARY KEY (device_id, account_id),
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS sessions (
                token TEXT PRIMARY KEY,
                device_id TEXT NOT NULL,
                account_id TEXT NOT NULL,
                firmware_version TEXT,
                capabilities_json TEXT NOT NULL,
                created_at INTEGER NOT NULL,
                expires_at INTEGER NOT NULL,
                FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE,
                FOREIGN KEY (account_id) REFERENCES accounts(account_id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS user_sessions (
                token TEXT PRIMARY KEY,
                user_id INTEGER NOT NULL,
                created_at INTEGER NOT NULL,
                expires_at INTEGER NOT NULL,
                FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS admin_sessions (
                token TEXT PRIMARY KEY,
                admin_id INTEGER NOT NULL,
                created_at INTEGER NOT NULL,
                expires_at INTEGER NOT NULL,
                FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS spin_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                request_id TEXT,
                device_id TEXT NOT NULL,
                account_id TEXT NOT NULL,
                mode TEXT NOT NULL,
                bet INTEGER NOT NULL,
                payout_multiplier INTEGER NOT NULL,
                payout INTEGER NOT NULL,
                balance_after INTEGER NOT NULL,
                symbols_json TEXT NOT NULL,
                created_at INTEGER NOT NULL
            );
            CREATE UNIQUE INDEX IF NOT EXISTS idx_spin_request
              ON spin_history(request_id, device_id, account_id, mode, bet)
              WHERE request_id IS NOT NULL;
            CREATE TABLE IF NOT EXISTS event_log (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id TEXT NOT NULL,
                account_id TEXT,
                event_type TEXT NOT NULL,
                value INTEGER,
                message TEXT,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS settings (
                key TEXT PRIMARY KEY,
                value TEXT NOT NULL,
                updated_at INTEGER NOT NULL
            );
            """
        )
        now = int(time.time())
        for key, value in DEFAULT_SETTINGS.items():
            conn.execute(
                "INSERT OR IGNORE INTO settings(key, value, updated_at) VALUES (?, ?, ?)",
                (key, str(value), now),
            )
        row = conn.execute("SELECT id FROM admins WHERE username=? LIMIT 1", (SUPERUSER_USERNAME,)).fetchone()
        if row is None:
            conn.execute(
                "INSERT INTO admins(username, password_hash, is_superuser, created_at) VALUES (?, ?, 1, ?)",
                (SUPERUSER_USERNAME, hash_password(SUPERUSER_PASSWORD), now),
            )


def get_settings(conn: sqlite3.Connection) -> Dict[str, float]:
    rows = conn.execute("SELECT key, value FROM settings").fetchall()
    result: Dict[str, float] = {}
    for row in rows:
        k = str(row["key"])
        v = str(row["value"])
        try:
            result[k] = float(v)
        except ValueError:
            result[k] = float(DEFAULT_SETTINGS.get(k, 0.0))
    for k, v in DEFAULT_SETTINGS.items():
        if k not in result:
            result[k] = float(v)
    return result


def upsert_device(conn: sqlite3.Connection, device_id: str) -> None:
    conn.execute(
        "INSERT OR IGNORE INTO devices(device_id, created_at) VALUES (?, ?)",
        (device_id, int(time.time())),
    )


def ensure_balance(conn: sqlite3.Connection, device_id: str, account_id: str) -> int:
    row = conn.execute(
        "SELECT balance FROM balances WHERE device_id=? AND account_id=?",
        (device_id, account_id),
    ).fetchone()
    if row:
        return int(row["balance"])
    initial = int(get_settings(conn)["initial_balance"])
    conn.execute(
        "INSERT INTO balances(device_id, account_id, balance, updated_at) VALUES (?, ?, ?, ?)",
        (device_id, account_id, initial, int(time.time())),
    )
    return initial


def set_balance(conn: sqlite3.Connection, device_id: str, account_id: str, value: int) -> None:
    conn.execute(
        """
        INSERT INTO balances(device_id, account_id, balance, updated_at)
        VALUES (?, ?, ?, ?)
        ON CONFLICT(device_id, account_id)
        DO UPDATE SET balance=excluded.balance, updated_at=excluded.updated_at
        """,
        (device_id, account_id, value, int(time.time())),
    )


def ensure_stub_account(conn: sqlite3.Connection, device_id: str) -> None:
    # MVP behavior: do NOT auto-create anonymous/default accounts for a device.
    # This function is kept for compatibility with existing call sites, but now
    # it only registers the device itself. Existing DB accounts are untouched.
    upsert_device(conn, device_id)


def list_device_accounts(conn: sqlite3.Connection, device_id: str) -> List[Dict[str, Any]]:
    ensure_stub_account(conn, device_id)
    rows = conn.execute(
        """
        SELECT a.account_id, a.display_name, b.balance
        FROM device_accounts da
        JOIN accounts a ON a.account_id = da.account_id
        LEFT JOIN balances b ON b.device_id = da.device_id AND b.account_id = da.account_id
        WHERE da.device_id=?
        ORDER BY a.created_at ASC
        """,
        (device_id,),
    ).fetchall()
    out: List[Dict[str, Any]] = []
    for row in rows:
        account_id = str(row["account_id"])
        out.append(
            {
                "accountId": account_id,
                "displayName": str(row["display_name"]),
                "balance": int(row["balance"]) if row["balance"] is not None else ensure_balance(conn, device_id, account_id),
            }
        )
    return out


def create_device_session(
    conn: sqlite3.Connection, device_id: str, account_id: str, firmware_version: str, capabilities: List[str]
) -> str:
    token = secrets.token_urlsafe(24)
    now = int(time.time())
    conn.execute("DELETE FROM sessions WHERE device_id=?", (device_id,))
    conn.execute(
        """
        INSERT INTO sessions(token, device_id, account_id, firmware_version, capabilities_json, created_at, expires_at)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
        (token, device_id, account_id, firmware_version, json.dumps(capabilities), now, now + SESSION_TTL_SEC),
    )
    return token


def create_admin_session(conn: sqlite3.Connection, admin_id: int) -> str:
    token = secrets.token_urlsafe(32)
    now = int(time.time())
    conn.execute("DELETE FROM admin_sessions WHERE admin_id=?", (admin_id,))
    conn.execute(
        "INSERT INTO admin_sessions(token, admin_id, created_at, expires_at) VALUES (?, ?, ?, ?)",
        (token, admin_id, now, now + ADMIN_SESSION_TTL_SEC),
    )
    return token


def create_user_portal_session(conn: sqlite3.Connection, user_id: int) -> str:
    token = secrets.token_urlsafe(32)
    now = int(time.time())
    conn.execute("DELETE FROM user_sessions WHERE user_id=?", (user_id,))
    conn.execute(
        "INSERT INTO user_sessions(token, user_id, created_at, expires_at) VALUES (?, ?, ?, ?)",
        (token, user_id, now, now + USER_PORTAL_SESSION_TTL_SEC),
    )
    return token


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


def bearer_token_from_header(request: web.Request) -> str:
    auth_header = request.headers.get("Authorization", "").strip()
    if not auth_header.lower().startswith("bearer "):
        return ""
    return auth_header[7:].strip()


def require_admin(conn: sqlite3.Connection, request: web.Request) -> Tuple[sqlite3.Row | None, web.Response | None]:
    token = bearer_token_from_header(request) or request.headers.get("X-Admin-Token", "").strip()
    if not token:
        return None, error_resp(401, "unauthorized", "Admin token required")
    row = conn.execute(
        """
        SELECT a.*
        FROM admin_sessions s
        JOIN admins a ON a.id=s.admin_id
        WHERE s.token=? AND s.expires_at>=?
        LIMIT 1
        """,
        (token, int(time.time())),
    ).fetchone()
    if row is None:
        return None, error_resp(401, "invalid_session", "Admin session invalid")
    return row, None


def require_user_portal(conn: sqlite3.Connection, request: web.Request) -> Tuple[sqlite3.Row | None, web.Response | None]:
    token = bearer_token_from_header(request)
    if not token:
        return None, error_resp(401, "unauthorized", "User token required")
    row = conn.execute(
        """
        SELECT u.*
        FROM user_sessions s
        JOIN users u ON u.id=s.user_id
        WHERE s.token=? AND s.expires_at>=?
        LIMIT 1
        """,
        (token, int(time.time())),
    ).fetchone()
    if row is None:
        return None, error_resp(401, "invalid_session", "User session invalid")
    return row, None


def verify_device_session(payload: Dict[str, Any]) -> Tuple[str, sqlite3.Row | None, web.Response | None]:
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return "", None, error_resp(400, "validation_error", err)
    token = payload.get("sessionToken")
    if token is None or not isinstance(token, str) or not token.strip():
        return "", None, error_resp(401, "unauthorized", "Missing or invalid session token")
    with db_conn() as conn:
        session = conn.execute(
            "SELECT * FROM sessions WHERE token=? AND device_id=? LIMIT 1",
            (token.strip(), device_id),
        ).fetchone()
        if session is None or int(session["expires_at"]) < int(time.time()):
            return "", None, error_resp(401, "invalid_session", "Session is not valid for this device")
        return device_id, session, None


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(value, high))


def random_symbol() -> int:
    return random.randrange(SYMBOL_MODULO)


def random_grid(rows: int, reels: int) -> List[List[int]]:
    return [[random_symbol() for _ in range(reels)] for _ in range(rows)]


def payout_multiplier(symbols: List[List[int]], mode: str) -> int:
    if mode == "3-reels":
        return 10 if symbols[0][0] == symbols[0][1] == symbols[0][2] else 0
    total = 0
    for line in PAYLINES:
        anchor = symbols[line[0]][0]
        matched = 1
        for reel in range(1, 5):
            if symbols[line[reel]][reel] != anchor:
                break
            matched += 1
        if matched >= 5:
            total += 10
        elif matched == 4:
            total += 5
        elif matched == 3:
            total += 2
    return total


def generate_3_reels(should_win: bool) -> List[List[int]]:
    if should_win:
        symbol = random_symbol()
        return [[symbol, symbol, symbol]]
    row = [random_symbol(), random_symbol(), random_symbol()]
    if row[0] == row[1] == row[2]:
        row[2] = (row[2] + 1) % SYMBOL_MODULO
    return [row]


def break_any_3plus_chains_5(symbols: List[List[int]]) -> List[List[int]]:
    for _ in range(20):
        changed = False
        for line in PAYLINES:
            if symbols[line[0]][0] == symbols[line[1]][1] == symbols[line[2]][2]:
                symbols[line[2]][2] = (symbols[line[2]][2] + 1) % SYMBOL_MODULO
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
        return break_any_3plus_chains_5(random_grid(rows=3, reels=5))
    symbols = random_grid(rows=3, reels=5)
    line = random.choice(PAYLINES)
    symbol = random_symbol()
    roll = random.random()
    win_len = 3 if roll < 0.60 else 4 if roll < 0.90 else 5
    for reel in range(win_len):
        symbols[line[reel]][reel] = symbol
    return symbols


async def health(_request: web.Request) -> web.Response:
    return json_resp(200, {"status": "ok", "message": "backend is healthy"})


async def list_accounts(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")
    with db_conn() as conn:
        accounts = list_device_accounts(conn, device_id)
    return json_resp(200, {"status": "ok", "accounts": accounts, "message": "accounts"})


async def auth(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    account_id, err = extract_non_empty_str(payload, "accountId")
    if err:
        return error_resp(400, "validation_error", err)
    firmware_version = str(payload.get("firmwareVersion") or "iot_terminal_ui")[:32]
    capabilities = payload.get("capabilities", [])
    if not isinstance(capabilities, list) or any(not isinstance(c, str) for c in capabilities):
        return error_resp(400, "validation_error", "Field 'capabilities' must be an array of strings")
    with db_conn() as conn:
        accounts = list_device_accounts(conn, device_id)
        account = next((x for x in accounts if x["accountId"] == account_id), None)
        if account is None:
            return error_resp(404, "account_not_found", "Requested account is not available for this device")
        token = create_device_session(conn, device_id, account_id, firmware_version, capabilities)
        conn.execute(
            "INSERT INTO event_log(device_id, account_id, event_type, value, message, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (device_id, account_id, "device_auth", None, "auth ok", int(time.time())),
        )
    return json_resp(
        200,
        {
            "status": "ok",
            "authorized": True,
            "accountId": account_id,
            "token": token,
            "userName": account["displayName"],
            "message": "auth ok",
        },
    )


async def balance(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")
    token = bearer_token_from_header(request)
    if not token:
        return error_resp(401, "unauthorized", "Missing bearer token")
    with db_conn() as conn:
        session = conn.execute(
            "SELECT * FROM sessions WHERE token=? AND device_id=? LIMIT 1", (token, device_id)
        ).fetchone()
        if session is None or int(session["expires_at"]) < int(time.time()):
            return error_resp(401, "invalid_session", "Session is not valid for this device")
        account_id = str(session["account_id"])
        value = ensure_balance(conn, device_id, account_id)
    return json_resp(200, {"status": "ok", "balance": value, "currency": DEFAULT_CURRENCY, "message": "balance"})


async def run_spin(request: web.Request, mode: str) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    if "sessionToken" not in payload:
        header_token = bearer_token_from_header(request)
        if header_token:
            payload["sessionToken"] = header_token
    device_id, session, err_resp = verify_device_session(payload)
    if err_resp:
        return err_resp
    if payload.get("mode") != mode:
        return error_resp(400, "validation_error", f"Field 'mode' must be '{mode}'")
    bet, err = validate_bet(payload)
    if err:
        return error_resp(400, "validation_error", err)
    if "clientBalance" in payload and (not isinstance(payload.get("clientBalance"), int) or payload.get("clientBalance") < 0):
        return error_resp(400, "validation_error", "Field 'clientBalance' must be a non-negative integer")
    account_id = str(session["account_id"])
    request_id = payload.get("requestId")
    if request_id is not None and (not isinstance(request_id, str) or not request_id.strip()):
        return error_resp(400, "validation_error", "Field 'requestId' must be a non-empty string")
    request_id = request_id.strip() if isinstance(request_id, str) else None

    with db_conn() as conn:
        current_balance = ensure_balance(conn, device_id, account_id)
        if current_balance < bet:
            return error_resp(409, "insufficient_balance", "balance is too low")
        if request_id:
            existing = conn.execute(
                """
                SELECT payout_multiplier, balance_after, symbols_json
                FROM spin_history
                WHERE request_id=? AND device_id=? AND account_id=? AND mode=? AND bet=?
                LIMIT 1
                """,
                (request_id, device_id, account_id, mode, bet),
            ).fetchone()
            if existing:
                return json_resp(
                    200,
                    {
                        "status": "ok",
                        "mode": mode,
                        "symbols": json.loads(str(existing["symbols_json"])),
                        "payoutMultiplier": int(existing["payout_multiplier"]),
                        "balance": int(existing["balance_after"]),
                        "message": "spin ok (deduplicated)",
                    },
                )

        settings = get_settings(conn)
        base = settings["base_win_chance_3"] if mode == "3-reels" else settings["base_win_chance_5"]
        cap = settings["max_win_chance_3"] if mode == "3-reels" else settings["max_win_chance_5"]
        recent_losses = conn.execute(
            """
            SELECT COUNT(*) AS cnt
            FROM spin_history
            WHERE device_id=? AND account_id=? AND mode=? AND payout_multiplier=0
            AND created_at>=?
            """,
            (device_id, account_id, mode, int(time.time()) - 3600),
        ).fetchone()
        chance = clamp((base + int(recent_losses["cnt"]) * settings["loss_streak_step"]) * settings["win_bias"], 0.0, cap)
        should_win = random.random() < chance
        symbols = generate_3_reels(should_win) if mode == "3-reels" else generate_5_reels(should_win)
        multiplier = payout_multiplier(symbols, mode)
        payout = bet * multiplier
        new_balance = max(0, current_balance - bet + payout)
        set_balance(conn, device_id, account_id, new_balance)
        conn.execute(
            """
            INSERT INTO spin_history(request_id, device_id, account_id, mode, bet, payout_multiplier, payout, balance_after, symbols_json, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (request_id, device_id, account_id, mode, bet, multiplier, payout, new_balance, json.dumps(symbols), int(time.time())),
        )
        conn.execute(
            "INSERT INTO event_log(device_id, account_id, event_type, value, message, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (device_id, account_id, "spin", new_balance, f"{mode}:{multiplier}", int(time.time())),
        )

    return json_resp(
        200,
        {"status": "ok", "mode": mode, "symbols": symbols, "payoutMultiplier": multiplier, "balance": new_balance, "message": "spin ok"},
    )


async def spin_3(request: web.Request) -> web.Response:
    return await run_spin(request, "3-reels")


async def spin_5(request: web.Request) -> web.Response:
    return await run_spin(request, "5-reels")


async def device_events(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    if "sessionToken" not in payload:
        header = bearer_token_from_header(request)
        if header:
            payload["sessionToken"] = header
    device_id, session, err_resp = verify_device_session(payload)
    if err_resp:
        return err_resp
    event_type, err = extract_non_empty_str(payload, "eventType")
    if err:
        return error_resp(400, "validation_error", err)
    value = payload.get("value")
    if value is not None and (not isinstance(value, int) or value < 0):
        return error_resp(400, "validation_error", "Field 'value' must be a non-negative integer")
    account_id = str(session["account_id"])
    with db_conn() as conn:
        if value is not None and event_type == "sync_balance":
            set_balance(conn, device_id, account_id, value)
        conn.execute(
            "INSERT INTO event_log(device_id, account_id, event_type, value, message, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (device_id, account_id, event_type, value, str(payload.get("message", ""))[:128], int(time.time())),
        )
    return json_resp(202, {"status": "ok", "message": f"Event '{event_type}' accepted"})


async def register_user(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    password, err = extract_non_empty_str(payload, "password")
    if err:
        return error_resp(400, "validation_error", err)
    display_name = str(payload.get("displayName") or username)[:32]
    device_id = str(payload.get("deviceId") or "").strip()
    initial_balance = payload.get("initialBalance", 0)
    with db_conn() as conn:
        username_norm = username.strip().lower()
        if conn.execute("SELECT 1 FROM users WHERE username=? LIMIT 1", (username_norm,)).fetchone():
            return error_resp(409, "user_exists", "User with this username already exists")
        now = int(time.time())
        conn.execute(
            "INSERT INTO users(username, password_hash, created_at) VALUES (?, ?, ?)",
            (username_norm, hash_password(password), now),
        )
        user_id = int(conn.execute("SELECT id FROM users WHERE username=?", (username_norm,)).fetchone()["id"])
        account_id = generate_account_id()
        conn.execute(
            "INSERT INTO accounts(account_id, user_id, display_name, created_at) VALUES (?, ?, ?, ?)",
            (account_id, user_id, display_name, now),
        )
        if device_id:
            upsert_device(conn, device_id)
            conn.execute(
                "INSERT INTO device_accounts(device_id, account_id, created_at) VALUES (?, ?, ?)",
                (device_id, account_id, now),
            )
            if not isinstance(initial_balance, int) or initial_balance < 0:
                return error_resp(400, "validation_error", "Field 'initialBalance' must be non-negative integer")
            set_balance(conn, device_id, account_id, initial_balance)
    return json_resp(201, {"status": "ok", "username": username_norm, "accountId": account_id, "message": "user registered"})


async def backend_login(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    password, err = extract_non_empty_str(payload, "password")
    if err:
        return error_resp(400, "validation_error", err)
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    firmware_version = str(payload.get("firmwareVersion") or "backend-login")[:32]
    capabilities = payload.get("capabilities", [])
    if not isinstance(capabilities, list) or any(not isinstance(c, str) for c in capabilities):
        return error_resp(400, "validation_error", "Field 'capabilities' must be an array of strings")
    with db_conn() as conn:
        row = conn.execute(
            """
            SELECT u.id AS user_id, u.password_hash, a.account_id, a.display_name
            FROM users u
            JOIN accounts a ON a.user_id=u.id
            WHERE u.username=?
            ORDER BY a.created_at ASC
            LIMIT 1
            """,
            (username.strip().lower(),),
        ).fetchone()
        if row is None or not verify_password(password, str(row["password_hash"])):
            return error_resp(401, "unauthorized", "Invalid username or password")
        account_id = str(row["account_id"])
        upsert_device(conn, device_id)
        conn.execute(
            "INSERT OR IGNORE INTO device_accounts(device_id, account_id, created_at) VALUES (?, ?, ?)",
            (device_id, account_id, int(time.time())),
        )
        ensure_balance(conn, device_id, account_id)
        game_token = create_device_session(conn, device_id, account_id, firmware_version, capabilities)
        user_token = create_user_portal_session(conn, int(row["user_id"]))
    return json_resp(
        200,
        {
            "status": "ok",
            "authorized": True,
            "accountId": account_id,
            "token": game_token,
            "userToken": user_token,
            "userName": str(row["display_name"]),
            "message": "auth ok",
        },
    )


async def user_portal_login(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    password, err = extract_non_empty_str(payload, "password")
    if err:
        return error_resp(400, "validation_error", err)
    with db_conn() as conn:
        row = conn.execute("SELECT * FROM users WHERE username=? LIMIT 1", (username.strip().lower(),)).fetchone()
        if row is None or not verify_password(password, str(row["password_hash"])):
            return error_resp(401, "unauthorized", "Invalid username or password")
        token = create_user_portal_session(conn, int(row["id"]))
    return json_resp(200, {"status": "ok", "token": token, "message": "portal login ok"})


async def user_create_account(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    display_name = str(payload.get("displayName") or "Player")[:32]
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    initial_balance = payload.get("initialBalance", 0)
    if not isinstance(initial_balance, int) or initial_balance < 0:
        return error_resp(400, "validation_error", "Field 'initialBalance' must be non-negative integer")
    with db_conn() as conn:
        user, auth_err = require_user_portal(conn, request)
        if auth_err:
            return auth_err
        account_id = generate_account_id()
        now = int(time.time())
        conn.execute(
            "INSERT INTO accounts(account_id, user_id, display_name, created_at) VALUES (?, ?, ?, ?)",
            (account_id, int(user["id"]), display_name, now),
        )
        upsert_device(conn, device_id)
        conn.execute(
            "INSERT INTO device_accounts(device_id, account_id, created_at) VALUES (?, ?, ?)",
            (device_id, account_id, now),
        )
        set_balance(conn, device_id, account_id, initial_balance)
        accounts = list_device_accounts(conn, device_id)
    return json_resp(201, {"status": "ok", "accountId": account_id, "deviceId": device_id, "accountsOnDevice": accounts})


async def user_list_device_accounts(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")
    with db_conn() as conn:
        user, auth_err = require_user_portal(conn, request)
        if auth_err:
            return auth_err
        rows = conn.execute(
            """
            SELECT a.account_id, a.display_name, b.balance
            FROM device_accounts da
            JOIN accounts a ON a.account_id=da.account_id
            LEFT JOIN balances b ON b.device_id=da.device_id AND b.account_id=da.account_id
            WHERE da.device_id=? AND a.user_id=?
            ORDER BY a.created_at ASC
            """,
            (device_id, int(user["id"])),
        ).fetchall()
        accounts = [{"accountId": str(r["account_id"]), "displayName": str(r["display_name"]), "balance": int(r["balance"] or 0)} for r in rows]
    return json_resp(200, {"status": "ok", "accounts": accounts, "message": "user device accounts"})


async def user_topup(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    account_id, err = extract_non_empty_str(payload, "accountId")
    if err:
        return error_resp(400, "validation_error", err)
    topup_type, err = extract_non_empty_str(payload, "type")
    if err:
        return error_resp(400, "validation_error", err)

    if topup_type == "housing":
        bonus = 1000
        details = str(payload.get("address") or "")
    elif topup_type == "macbook":
        bonus = 500
        details = str(payload.get("version") or "")
    else:
        return error_resp(400, "validation_error", "Field 'type' must be 'housing' or 'macbook'")

    with db_conn() as conn:
        user, auth_err = require_user_portal(conn, request)
        if auth_err:
            return auth_err
        own = conn.execute(
            """
            SELECT 1
            FROM device_accounts da
            JOIN accounts a ON a.account_id=da.account_id
            WHERE da.device_id=? AND da.account_id=? AND a.user_id=?
            LIMIT 1
            """,
            (device_id, account_id, int(user["id"])),
        ).fetchone()
        if own is None:
            return error_resp(403, "forbidden", "Account is not available for this user")
        current = ensure_balance(conn, device_id, account_id)
        new_balance = current + bonus
        set_balance(conn, device_id, account_id, new_balance)
        conn.execute(
            "INSERT INTO event_log(device_id, account_id, event_type, value, message, created_at) VALUES (?, ?, ?, ?, ?, ?)",
            (device_id, account_id, f"topup_{topup_type}", bonus, details[:128], int(time.time())),
        )
    return json_resp(200, {"status": "ok", "bonus": bonus, "balance": new_balance, "message": "topup applied"})


async def user_account_stats(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    account_id = request.match_info.get("accountId", "").strip()
    if not device_id or not account_id:
        return error_resp(400, "validation_error", "Path parameters 'deviceId' and 'accountId' are required")
    with db_conn() as conn:
        user, auth_err = require_user_portal(conn, request)
        if auth_err:
            return auth_err
        own = conn.execute(
            """
            SELECT 1
            FROM device_accounts da
            JOIN accounts a ON a.account_id=da.account_id
            WHERE da.device_id=? AND da.account_id=? AND a.user_id=?
            LIMIT 1
            """,
            (device_id, account_id, int(user["id"])),
        ).fetchone()
        if own is None:
            return error_resp(403, "forbidden", "Account is not available for this user")
        balance = ensure_balance(conn, device_id, account_id)
        agg = conn.execute(
            """
            SELECT
              COUNT(*) AS spins,
              COALESCE(SUM(bet),0) AS total_bet,
              COALESCE(SUM(payout),0) AS total_payout,
              COALESCE(SUM(CASE WHEN payout_multiplier>0 THEN 1 ELSE 0 END),0) AS wins
            FROM spin_history
            WHERE device_id=? AND account_id=?
            """,
            (device_id, account_id),
        ).fetchone()
        recent = conn.execute(
            """
            SELECT mode, bet, payout_multiplier, payout, balance_after, created_at
            FROM spin_history
            WHERE device_id=? AND account_id=?
            ORDER BY created_at DESC
            LIMIT 100
            """,
            (device_id, account_id),
        ).fetchall()
    spins = int(agg["spins"])
    wins = int(agg["wins"])
    win_rate = (wins / spins) if spins else 0.0
    return json_resp(
        200,
        {
            "status": "ok",
            "deviceId": device_id,
            "accountId": account_id,
            "balance": balance,
            "stats": {
                "spins": spins,
                "wins": wins,
                "winRate": round(win_rate, 4),
                "totalBet": int(agg["total_bet"]),
                "totalPayout": int(agg["total_payout"]),
            },
            "recentSpins": [dict(r) for r in recent],
        },
    )


async def user_panel(_request: web.Request) -> web.Response:
    if not USER_PANEL_PATH.exists():
        return error_resp(500, "user_panel_missing", "User panel template not found")
    return web.FileResponse(path=USER_PANEL_PATH)


async def admin_login(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    password, err = extract_non_empty_str(payload, "password")
    if err:
        return error_resp(400, "validation_error", err)
    with db_conn() as conn:
        row = conn.execute("SELECT * FROM admins WHERE username=? LIMIT 1", (username.strip().lower(),)).fetchone()
        if row is None or not verify_password(password, str(row["password_hash"])):
            return error_resp(401, "unauthorized", "Invalid admin credentials")
        token = create_admin_session(conn, int(row["id"]))
    return json_resp(
        200,
        {"status": "ok", "token": token, "admin": {"username": str(row["username"]), "isSuperuser": bool(row["is_superuser"])}},
    )


async def admin_me(request: web.Request) -> web.Response:
    with db_conn() as conn:
        admin, err = require_admin(conn, request)
        if err:
            return err
    return json_resp(200, {"status": "ok", "admin": {"username": str(admin["username"]), "isSuperuser": bool(admin["is_superuser"])}})


async def admin_create_admin(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    password, err = extract_non_empty_str(payload, "password")
    if err:
        return error_resp(400, "validation_error", err)
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        if int(admin["is_superuser"]) != 1:
            return error_resp(403, "forbidden", "Only superuser can create admins")
        uname = username.strip().lower()
        if conn.execute("SELECT 1 FROM admins WHERE username=? LIMIT 1", (uname,)).fetchone():
            return error_resp(409, "admin_exists", "Admin already exists")
        conn.execute(
            "INSERT INTO admins(username, password_hash, is_superuser, created_at) VALUES (?, ?, 0, ?)",
            (uname, hash_password(password), int(time.time())),
        )
    return json_resp(201, {"status": "ok", "message": "admin created", "username": uname})


async def admin_create_user(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
    return await register_user(request)


async def admin_create_account(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    username, err = extract_non_empty_str(payload, "username")
    if err:
        return error_resp(400, "validation_error", err)
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    display_name = str(payload.get("displayName") or username)[:32]
    initial_balance = payload.get("initialBalance")
    if initial_balance is not None and (not isinstance(initial_balance, int) or initial_balance < 0):
        return error_resp(400, "validation_error", "Field 'initialBalance' must be non-negative integer")
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
        user = conn.execute("SELECT id FROM users WHERE username=? LIMIT 1", (username.strip().lower(),)).fetchone()
        if user is None:
            return error_resp(404, "user_not_found", "User not found")
        account_id = generate_account_id()
        now = int(time.time())
        conn.execute(
            "INSERT INTO accounts(account_id, user_id, display_name, created_at) VALUES (?, ?, ?, ?)",
            (account_id, int(user["id"]), display_name, now),
        )
        upsert_device(conn, device_id)
        conn.execute(
            "INSERT INTO device_accounts(device_id, account_id, created_at) VALUES (?, ?, ?)",
            (device_id, account_id, now),
        )
        if initial_balance is not None:
            set_balance(conn, device_id, account_id, initial_balance)
        else:
            ensure_balance(conn, device_id, account_id)
        accounts = list_device_accounts(conn, device_id)
    return json_resp(201, {"status": "ok", "accountId": account_id, "accountsOnDevice": accounts})


async def admin_set_balance(request: web.Request) -> web.Response:
    payload, err_resp = await read_json(request)
    if err_resp:
        return err_resp
    device_id, err = extract_non_empty_str(payload, "deviceId")
    if err:
        return error_resp(400, "validation_error", err)
    account_id, err = extract_non_empty_str(payload, "accountId")
    if err:
        return error_resp(400, "validation_error", err)
    value = payload.get("balance")
    if not isinstance(value, int) or value < 0:
        return error_resp(400, "validation_error", "Field 'balance' must be non-negative integer")
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
        set_balance(conn, device_id, account_id, value)
    return json_resp(200, {"status": "ok", "message": "balance updated"})


async def admin_luck_settings(request: web.Request) -> web.Response:
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
        if request.method == "GET":
            return json_resp(200, {"status": "ok", "settings": get_settings(conn)})
        payload, err_resp = await read_json(request)
        if err_resp:
            return err_resp
        allowed = set(DEFAULT_SETTINGS.keys()) - {"initial_balance"}
        for key, val in payload.items():
            if key not in allowed:
                continue
            if not isinstance(val, (int, float)):
                return error_resp(400, "validation_error", f"Setting '{key}' must be number")
            conn.execute(
                """
                INSERT INTO settings(key, value, updated_at) VALUES (?, ?, ?)
                ON CONFLICT(key) DO UPDATE SET value=excluded.value, updated_at=excluded.updated_at
                """,
                (key, str(val), int(time.time())),
            )
        return json_resp(200, {"status": "ok", "settings": get_settings(conn), "message": "settings updated"})


async def admin_overview(request: web.Request) -> web.Response:
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
        users_rows = conn.execute(
            """
            SELECT u.id AS user_id, u.username, a.account_id, a.display_name
            FROM users u
            LEFT JOIN accounts a ON a.user_id=u.id
            ORDER BY u.created_at DESC, a.created_at ASC
            """
        ).fetchall()
        admins_rows = conn.execute("SELECT username, is_superuser, created_at FROM admins ORDER BY created_at DESC").fetchall()
        balances = conn.execute("SELECT device_id, account_id, balance, updated_at FROM balances ORDER BY updated_at DESC LIMIT 500").fetchall()
        sessions = conn.execute("SELECT device_id, account_id, firmware_version, created_at, expires_at FROM sessions ORDER BY created_at DESC LIMIT 200").fetchall()
        spins = conn.execute(
            "SELECT id, request_id, device_id, account_id, mode, bet, payout_multiplier, payout, balance_after, created_at FROM spin_history ORDER BY created_at DESC LIMIT 500"
        ).fetchall()
        events = conn.execute(
            "SELECT id, device_id, account_id, event_type, value, message, created_at FROM event_log ORDER BY created_at DESC LIMIT 500"
        ).fetchall()
        settings = get_settings(conn)

    users_map: Dict[int, Dict[str, Any]] = {}
    for row in users_rows:
        uid = int(row["user_id"])
        if uid not in users_map:
            users_map[uid] = {"userId": uid, "username": str(row["username"]), "accounts": []}
        if row["account_id"] is not None:
            users_map[uid]["accounts"].append({"accountId": str(row["account_id"]), "displayName": str(row["display_name"])})
    return json_resp(
        200,
        {
            "status": "ok",
            "summary": {
                "users": len(users_map),
                "admins": len(admins_rows),
                "balances": len(balances),
                "activeSessions": len(sessions),
                "recentSpins": len(spins),
                "recentEvents": len(events),
            },
            "users": list(users_map.values()),
            "admins": [dict(x) for x in admins_rows],
            "balances": [dict(x) for x in balances],
            "sessions": [dict(x) for x in sessions],
            "spins": [dict(x) for x in spins],
            "events": [dict(x) for x in events],
            "settings": settings,
        },
    )


async def admin_device_accounts(request: web.Request) -> web.Response:
    device_id = request.match_info.get("deviceId", "").strip()
    if not device_id:
        return error_resp(400, "validation_error", "Path parameter 'deviceId' is required")
    with db_conn() as conn:
        admin, auth_err = require_admin(conn, request)
        if auth_err:
            return auth_err
        _ = admin
        accounts = list_device_accounts(conn, device_id)
    return json_resp(200, {"status": "ok", "deviceId": device_id, "accounts": accounts})


async def admin_panel(_request: web.Request) -> web.Response:
    if not ADMIN_PANEL_PATH.exists():
        return error_resp(500, "admin_panel_missing", "Admin panel template not found")
    return web.FileResponse(path=ADMIN_PANEL_PATH)


async def not_found(_request: web.Request) -> web.Response:
    return error_resp(404, "not_found", "Route not found")


def make_app() -> web.Application:
    init_db()
    app = web.Application(middlewares=[cors_middleware])
    app.router.add_get("/admin", admin_panel)
    app.router.add_get("/user", user_panel)
    app.router.add_get(f"{API_PREFIX}/health", health)
    app.router.add_post(f"{API_PREFIX}/backend/users/register", register_user)
    app.router.add_post(f"{API_PREFIX}/backend/users/login", backend_login)
    app.router.add_post(f"{API_PREFIX}/backend/users/portal/login", user_portal_login)
    app.router.add_post(f"{API_PREFIX}/backend/users/accounts", user_create_account)
    app.router.add_get(f"{API_PREFIX}/backend/users/devices/{{deviceId}}/accounts", user_list_device_accounts)
    app.router.add_post(f"{API_PREFIX}/backend/users/topup", user_topup)
    app.router.add_get(f"{API_PREFIX}/backend/users/stats/{{deviceId}}/{{accountId}}", user_account_stats)
    app.router.add_post(f"{API_PREFIX}/backend/admin/login", admin_login)
    app.router.add_get(f"{API_PREFIX}/backend/admin/me", admin_me)
    app.router.add_post(f"{API_PREFIX}/backend/admin/admins", admin_create_admin)
    app.router.add_post(f"{API_PREFIX}/backend/admin/users", admin_create_user)
    app.router.add_post(f"{API_PREFIX}/backend/admin/accounts", admin_create_account)
    app.router.add_patch(f"{API_PREFIX}/backend/admin/balance", admin_set_balance)
    app.router.add_get(f"{API_PREFIX}/backend/admin/settings/luck", admin_luck_settings)
    app.router.add_put(f"{API_PREFIX}/backend/admin/settings/luck", admin_luck_settings)
    app.router.add_get(f"{API_PREFIX}/backend/admin/devices/{{deviceId}}/accounts", admin_device_accounts)
    app.router.add_get(f"{API_PREFIX}/backend/admin/overview", admin_overview)
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
    parser = argparse.ArgumentParser(description="Backend server for iot_terminal_ui")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    app = make_app()
    print(f"Backend listening on http://{args.host}:{args.port}")
    print(f"SQLite database: {DB_PATH}")
    print(f"Superuser: {SUPERUSER_USERNAME}")
    print("Press Ctrl+C to stop.")
    web.run_app(app, host=args.host, port=args.port, access_log=None)


if __name__ == "__main__":
    main()
