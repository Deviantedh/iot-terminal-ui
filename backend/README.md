# Mock Backend

Tiny local server for checking what the device sends to the backend API.

Run from the project root:

```bash
python3 -m pip install -r backend/requirements.txt
python3 backend/mock_server.py --host 0.0.0.0 --port 8080
```

The async server returns JSON responses for health, account list, device auth,
account-scoped balance, 3-reels spin, 5-reels spin, and device events.

The backend now uses SQLite storage (`backend/backend.sqlite3` by default):

- users
- devices
- accounts
- device-account ownership
- sessions (token + TTL)
- balances per `deviceId + accountId`
- spin history and event log

## Backend and admin endpoints

User lifecycle and user self-service:

- `POST /api/v1/backend/users/register`
- `POST /api/v1/backend/users/login`
- `POST /api/v1/backend/users/portal/login`
- `POST /api/v1/backend/users/accounts` (requires user bearer token)
- `GET /api/v1/backend/users/devices/{deviceId}/accounts` (requires user bearer token)
- `POST /api/v1/backend/users/topup` (requires user bearer token)
- `GET /api/v1/backend/users/stats/{deviceId}/{accountId}` (requires user bearer token)
- `GET /user` (separate user panel)

Admin control center:

- `GET /admin` (HTML panel)
- `POST /api/v1/backend/admin/login`
- `GET /api/v1/backend/admin/me`
- `GET /api/v1/backend/admin/overview`
- `POST /api/v1/backend/admin/users`
- `POST /api/v1/backend/admin/accounts`
- `PATCH /api/v1/backend/admin/balance`
- `POST /api/v1/backend/admin/admins` (superuser only)
- `GET|PUT /api/v1/backend/admin/settings/luck`
- `GET /api/v1/backend/admin/devices/{deviceId}/accounts`

Default superuser credentials (override with env):

- `SUPERUSER_USERNAME=admin`
- `SUPERUSER_PASSWORD=admin123`

Session env vars:

- `SESSION_TTL_SEC`
- `ADMIN_SESSION_TTL_SEC`
- `USER_PORTAL_SESSION_TTL_SEC`

`accountId` is generated from UUIDv4 bytes (URL-safe base64, 22 chars) to avoid collisions while staying compact.

Firmware-compatible flow remains unchanged:

- `GET /api/v1/devices/{deviceId}/accounts`
- `POST /api/v1/devices/auth` (requires `accountId`)
- `GET /api/v1/devices/{deviceId}/balance`
- spin/events in session context

If no account is linked to a device yet, backend auto-creates one stub account
named `Player-{deviceId}` and binds it to that device.

## Run as a daemon on Linux

Copy the project to the server, for example:

```bash
sudo mkdir -p /opt/iot-terminal-ui
sudo cp -R . /opt/iot-terminal-ui
```

If you copy only the backend folder, keep the same inner path:

```bash
sudo mkdir -p /opt/iot-terminal-ui/backend
sudo rsync -av --delete backend/ /opt/iot-terminal-ui/backend/
```

Install and start the systemd service:

```bash
sudo /opt/iot-terminal-ui/backend/install_systemd_service.sh /opt/iot-terminal-ui
```

Useful commands:

```bash
sudo systemctl status iot-terminal-backend
sudo journalctl -u iot-terminal-backend -f
sudo systemctl restart iot-terminal-backend
sudo systemctl stop iot-terminal-backend
```

The server will keep running after SSH disconnects and will restart automatically
if the process exits.
