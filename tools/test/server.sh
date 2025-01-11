#!/bin/bash
set -euo pipefail

PROTOCOL="ip"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -l, --loss PERCENTAGE       Packet loss percentage"
    echo "  -c, --corruption PERCENTAGE Packet corruption percentage"
    echo "  -p, --protocol PROTOCOL     Protocol to use (ip/udp/tcp)"
    echo "  -h, --help                  Show this help message"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--protocol)
            PROTOCOL="$2"
            shift 2
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

echo "Starting server..."
docker compose exec server bash -c "cd /app && build/${PROTOCOL}/server"