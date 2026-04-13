# Mock Backend

Tiny local server for checking what the device sends to the backend API.

Run from the project root:

```bash
python3 backend/mock_server.py --host 0.0.0.0 --port 8080
```

The server prints every request to the terminal and returns small JSON responses
for health, auth, balance, 3-reels spin, 5-reels spin, and device events.

It keeps a tiny in-memory balance dictionary by `deviceId`. `spin_win` and
`spin_loss` events update that balance from the request `value`, which lets the
firmware sync balance back through the normal `/balance` endpoint.

## Run as a daemon on Linux

Copy the project to the server, for example:

```bash
sudo mkdir -p /opt/iot-terminal-ui
sudo cp -R . /opt/iot-terminal-ui
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
