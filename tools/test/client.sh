#!/bin/bash
set -euo pipefail

MESSAGE="Hello World"
PROTOCOL="udp"
DEST_PORT="12345"
SRC_PORT="12345"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -m, --message MESSAGE       Message to send"
    echo "  -p, --protocol PROTOCOL     Protocol to use (ip/udp)"
    echo "  -P, --port DEST_PORT        Server port"
    echo "  -sP, --src_port DEST_PORT   Client port"
    echo "  -h, --help                  Show this help message"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--message)
            MESSAGE="$2"
            shift 2
            ;;
        -p|--protocol)
            PROTOCOL="$2"
            shift 2
            ;;
        -P|--port)
            DEST_PORT="$2"
            shift 2
            ;;
        -sP|--src_port)
            SRC_PORT="$2"
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

echo "Getting server IP..."
SERVER_IP=$(docker compose exec client nslookup server | grep "Address" | tail -n1 | awk '{print $2}')

echo "Sending message from client process..."
if [ "$PROTOCOL" = "ip" ]; then
    docker compose exec client bash -c "cd /app && build/ip/client ${SERVER_IP} \"${MESSAGE}\""
else
    docker compose exec client bash -c "cd /app && build/udp/client ${SERVER_IP} ${DEST_PORT} \"${MESSAGE}\""
fi

echo "Test complete."