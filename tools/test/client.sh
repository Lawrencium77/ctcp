#!/bin/bash
set -euo pipefail

MESSAGE="Hello World"
PROTOCOL="ip"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -m, --message MESSAGE       Message to send"
    echo "  -p, --protocol PROTOCOL     Protocol to use (udp/tcp)"
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
docker compose exec client bash -c "cd /app && build/${PROTOCOL}/client ${SERVER_IP} \"${MESSAGE}\""

echo "Test complete."