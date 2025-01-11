#!/bin/bash
set -euo pipefail

PACKET_LOSS=0
PACKET_CORRUPTION=0
ACTION="set"

function show_help {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -l, --loss PERCENTAGE       Packet loss percentage"
    echo "  -c, --corruption PERCENTAGE Packet corruption percentage"
    echo "  -r, --reset                 Reset network conditions instead of setting them"
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
        -r|--reset)
            ACTION="reset"
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

function configure_container {
    local container=$1
    
    if [ "$ACTION" = "reset" ]; then
        echo "Resetting $container network conditions..."
        docker compose exec -T $container tc qdisc del dev eth0 root || true
    else
        echo "Configuring $container network conditions..."
        docker compose exec -T $container tc qdisc add dev eth0 root netem loss ${PACKET_LOSS}% corrupt ${PACKET_CORRUPTION}%
    fi
}

configure_container "server"
configure_container "client"