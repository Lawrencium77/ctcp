#!/bin/bash
set -euo pipefail

PACKET_LOSS=0
PACKET_CORRUPTION=0
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
        -l|--loss)
            PACKET_LOSS="$2"
            shift 2
            ;;
        -c|--corruption)
            PACKET_CORRUPTION="$2"
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

echo "Configuring server network conditions..."
docker compose exec -T server tc qdisc add dev eth0 root netem loss ${PACKET_LOSS}% corrupt ${PACKET_CORRUPTION}%

echo "Configuring client network conditions..."
docker compose exec -T client tc qdisc add dev eth0 root netem loss ${PACKET_LOSS}% corrupt ${PACKET_CORRUPTION}%

echo "Starting server..."
docker compose exec server bash -c "cd /app && build/${PROTOCOL}/server"