#!/bin/bash
set -euo pipefail

docker compose exec server bash -c "cd /app && make lint"