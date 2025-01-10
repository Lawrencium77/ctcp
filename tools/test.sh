#!/bin/bash
set -euo pipefail

PACKET_LOSS=50
PACKET_CORRUPTION=50
MESSAGE="Hello World"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -l, --loss PERCENTAGE       Set packet loss percentage (default: 50)"
    echo "  -c, --corruption PERCENTAGE Set packet corruption percentage (default: 50)"
    echo "  -m, --message MESSAGE       Message to send (default: 'Hello, World!')"
    echo "  -h, --help                  Show this help message"
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -l|--loss)
            PACKET_LOSS="$2"
            shift 2
            ;;
        -c|--corruption)
            PACKET_CORRUPTION="$2"
            shift 2
            ;;
        -m|--message)
            MESSAGE="$2"
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

cleanup() {
    echo "Cleaning up..."
    docker compose down
    exit 0
}

trap cleanup EXIT

echo "Starting containers..."
docker compose up -d

echo "Configuring server network conditions..."
docker compose exec -T server tc qdisc add dev eth0 root netem loss ${PACKET_LOSS}% corrupt ${PACKET_CORRUPTION}%

echo "Configuring client network conditions..."
docker compose exec -T client tc qdisc add dev eth0 root netem loss ${PACKET_LOSS}% corrupt ${PACKET_CORRUPTION}%

echo "Starting server..."
docker compose exec -d server bash -c "cd /app && build/udp/server"

echo "Getting server IP..."
SERVER_IP=$(docker compose exec -T client nslookup server | grep "Address" | tail -n1 | awk '{print $2}')

echo "Sending message from client process..."
docker compose exec -T client bash -c "cd /app && build/udp/client ${SERVER_IP} \"${MESSAGE}\""

echo "Test running. Press Ctrl+C to stop and cleanup."
while true; do
    sleep 1
done