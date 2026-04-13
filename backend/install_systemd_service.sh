#!/usr/bin/env bash
set -euo pipefail

SERVICE_NAME="iot-terminal-backend.service"
PROJECT_DIR="${1:-/opt/iot-terminal-ui}"
SERVICE_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/${SERVICE_NAME}"
SERVICE_DST="/etc/systemd/system/${SERVICE_NAME}"

if [[ ! -f "${PROJECT_DIR}/backend/mock_server.py" ]]; then
  echo "Backend file not found: ${PROJECT_DIR}/backend/mock_server.py" >&2
  echo "Copy the project to ${PROJECT_DIR}, or pass the project path as the first argument." >&2
  exit 1
fi

if [[ "${PROJECT_DIR}" != "/opt/iot-terminal-ui" ]]; then
  TMP_SERVICE="$(mktemp)"
  sed "s#/opt/iot-terminal-ui#${PROJECT_DIR}#g" "${SERVICE_SRC}" > "${TMP_SERVICE}"
  sudo install -m 0644 "${TMP_SERVICE}" "${SERVICE_DST}"
  rm -f "${TMP_SERVICE}"
else
  sudo install -m 0644 "${SERVICE_SRC}" "${SERVICE_DST}"
fi

sudo systemctl daemon-reload
sudo systemctl enable --now "${SERVICE_NAME}"
sudo systemctl status "${SERVICE_NAME}" --no-pager
