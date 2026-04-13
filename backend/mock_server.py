#!/usr/bin/env python3
import argparse
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse


class MockThreadingHTTPServer(ThreadingHTTPServer):
    allow_reuse_address = True
    daemon_threads = True
    request_queue_size = 32


class MockApiHandler(BaseHTTPRequestHandler):
    server_version = "iot-terminal-mock/0.1"
    protocol_version = "HTTP/1.0"

    balances = {}
    spin_counter = 0
    paylines = (
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

    def log_message(self, fmt, *args):
        return

    def _json_body(self, body):
        if not body:
            return {}
        try:
            return json.loads(body.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            return {}

    def _device_balance(self, device_id):
        if not device_id:
            device_id = "unknown"
        if device_id not in self.balances:
            self.balances[device_id] = 1000
        return self.balances[device_id]

    def _read_body(self):
        length = int(self.headers.get("Content-Length", "0") or "0")
        if length <= 0:
            return b""
        return self.rfile.read(length)

    def _send_json(self, status_code, payload):
        body = (json.dumps(payload, separators=(",", ":")) + "\n").encode("utf-8")
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)
        self.wfile.flush()

    def _payout_multiplier(self, symbols, reel_count, row_count):
        if reel_count == 3 or row_count < 3:
            return 10 if symbols[0][0] == symbols[0][1] == symbols[0][2] else 0

        multiplier = 0
        for line in self.paylines:
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

    def _apply_spin_balance(self, device_id, bet, symbols, reel_count, row_count):
        multiplier = self._payout_multiplier(symbols, reel_count, row_count)
        self.balances[device_id] = max(0, self._device_balance(device_id) - bet + bet * multiplier)
        return multiplier, self.balances[device_id]

    def _handle_request(self):
        body = self._read_body()
        path = urlparse(self.path).path
        payload = self._json_body(body)

        if path == "/api/v1/health":
            self._send_json(200, {
                "status": "ok",
                "message": "mock backend healthy",
            })
            return

        if path == "/api/v1/devices/auth":
            device_id = payload.get("deviceId") or "unknown"
            self._device_balance(device_id)
            self._send_json(200, {
                "status": "ok",
                "authorized": True,
                "token": f"mock-session-token-{device_id}",
                "userName": f"Player {device_id[-4:]}",
                "message": "mock auth ok",
            })
            return

        if path.endswith("/balance"):
            parts = path.strip("/").split("/")
            device_id = parts[3] if len(parts) >= 5 and parts[2] == "devices" else "unknown"
            self._send_json(200, {
                "status": "ok",
                "balance": self._device_balance(device_id),
                "currency": "USD",
                "message": "mock balance",
            })
            return

        if path == "/api/v1/games/slot/3-reels/spin":
            device_id = payload.get("deviceId") or "unknown"
            if "clientBalance" in payload:
                self.balances[device_id] = max(0, int(payload["clientBalance"]))
            bet = int(payload.get("bet") or 0)
            if self._device_balance(device_id) < bet:
                self._send_json(409, {
                    "status": "error",
                    "error": "insufficient_balance",
                    "message": "mock balance is too low",
                })
                return

            type(self).spin_counter += 1
            counter = type(self).spin_counter
            symbols = [[counter % 6, (counter + 1) % 6, (counter + 2) % 6]]
            multiplier, balance = self._apply_spin_balance(device_id, bet, symbols, 3, 1)

            self._send_json(200, {
                "status": "ok",
                "mode": "3-reels",
                "symbols": symbols,
                "payoutMultiplier": multiplier,
                "balance": balance,
                "message": "mock 3 reels",
            })
            return

        if path == "/api/v1/games/slot/5-reels/spin":
            device_id = payload.get("deviceId") or "unknown"
            if "clientBalance" in payload:
                self.balances[device_id] = max(0, int(payload["clientBalance"]))
            bet = int(payload.get("bet") or 0)
            if self._device_balance(device_id) < bet:
                self._send_json(409, {
                    "status": "error",
                    "error": "insufficient_balance",
                    "message": "mock balance is too low",
                })
                return

            type(self).spin_counter += 1
            offset = type(self).spin_counter % 6
            symbols = [
                [(offset + i) % 6 for i in range(5)],
                [(offset + i + 1) % 6 for i in range(5)],
                [(offset + i + 2) % 6 for i in range(5)],
            ]
            multiplier, balance = self._apply_spin_balance(device_id, bet, symbols, 5, 3)

            self._send_json(200, {
                "status": "ok",
                "mode": "5-reels",
                "symbols": symbols,
                "payoutMultiplier": multiplier,
                "balance": balance,
                "message": "mock 5 reels",
            })
            return

        if path == "/api/v1/devices/events":
            device_id = payload.get("deviceId") or "unknown"
            event_type = payload.get("eventType") or ""
            if event_type in ("spin_win", "spin_loss") and "value" in payload:
                self.balances[device_id] = max(0, int(payload["value"]))

            self._send_json(202, {
                "status": "ok",
                "message": "mock event accepted",
            })
            return

        self._send_json(200, {
            "status": "ok",
            "message": "mock default response",
        })

    def do_GET(self):
        self._handle_request()

    def do_POST(self):
        self._handle_request()


def main():
    parser = argparse.ArgumentParser(description="Tiny mock backend for iot_terminal_ui.")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    server = MockThreadingHTTPServer((args.host, args.port), MockApiHandler)
    print(f"Mock server listening on http://{args.host}:{args.port}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping mock server.")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()