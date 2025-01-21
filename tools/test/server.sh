#!/bin/bash
set -euo pipefail

PROTOCOL="udp"
PORT="12345"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -p, --protocol PROTOCOL     Protocol to use (ip/udp)"
    echo "  -P, --port PORT             Port to use"
    echo "  -h, --help                  Show this help message"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--protocol)
            PROTOCOL="$2"
            shift 2
            ;;
        -P|--port)
            PORT="$2"
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

case "${PROTOCOL}" in
    udp)
        echo "Starting ${PROTOCOL} server at port ${PORT}..."
        docker compose exec server bash -c "cd /app && build/udp/server ${PORT}"
        ;;
    ip)
        echo "Starting ${PROTOCOL} server..."
        docker compose exec server bash -c "cd /app && build/ip/server"
        ;;
    *)
        echo "Unknown protocol: ${PROTOCOL}"
        show_help
        ;;
esac