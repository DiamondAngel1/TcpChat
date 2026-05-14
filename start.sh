#!/bin/sh
set -e

BACKEND_PORT=${PORT:-10000}
export TCP_HOST=127.0.0.1
export TCP_PORT=${BACKEND_PORT}
if [ -f ./backend/server]; then
echo "[start.sh] Starting backend ./backend/server on port ${BACKEND_PORT}"
./backend/server &
BACKEND_PID=$!
else
echo "[start.sh] ERROR: backend/server not found. Exiting."
exit 1
fi
TRY=0
until nc -z 127.0.0.1 ${BACKEND_PORT} >/dev/null 2>&1 || [ $TRY -ge 50]; do
TRY=$((TRY+1))
echo "[start.sh] Waiting for backend to start... (${TRY})"
sleep 0.1
done

if [ $TRY -ge 50]; then
echo "[start.sh] Backend did not start in time. Exiting."
kill $BACKEND_PID || true
exit 1
fi
cd /app/proxy
echo "[start.sh] Starting proxy (npm start)"
npm start

Cleanup
kill $BACKEND_PID || true
wait $BACKEND_PID || true