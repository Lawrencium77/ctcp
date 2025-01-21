#!/bin/bash
set -euo pipefail

echo "Starting UDP daemon..."
docker compose exec server bash -c "cd /app && build/udp/daemon"