#!/bin/bash
set -euo pipefail

# Rebuild project code on server container.
# This updates binary files in client container too, via volume mount.
docker compose exec server bash -c "cd /app && make clean && make"
