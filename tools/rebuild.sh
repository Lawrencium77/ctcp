#!/bin/bash
set -euo pipefail

DEBUG=""

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --debug    Build with debug symbols"
    echo "  -h|--help  Show this help message"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            DEBUG="DEBUG=1"
            shift
            ;;
        -h|--help)
            show_help
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            ;;
    esac
done

# Rebuild project code on server container.
# This updates binary files in client container too, via volume mount.
docker compose exec server bash -c "cd /app && make clean && make ${DEBUG}"