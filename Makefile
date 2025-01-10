CC = gcc
BUILD_DIR = build
IP_DIR = src/ip
UDP_DIR = src/udp

all: ip udp

ip: $(BUILD_DIR)/ip/server $(BUILD_DIR)/ip/client

udp: $(BUILD_DIR)/udp/server $(BUILD_DIR)/udp/client

$(BUILD_DIR)/ip/server: $(IP_DIR)/server.c | $(BUILD_DIR)/ip
	$(CC) $(IP_DIR)/server.c -o $(BUILD_DIR)/ip/server -I $(IP_DIR)

$(BUILD_DIR)/ip/client: $(IP_DIR)/client.c | $(BUILD_DIR)/ip
	$(CC) $(IP_DIR)/client.c -o $(BUILD_DIR)/ip/client -I $(IP_DIR)

$(BUILD_DIR)/udp/server: $(UDP_DIR)/server.c | $(BUILD_DIR)/udp
	$(CC) $(UDP_DIR)/server.c -o $(BUILD_DIR)/udp/server -I $(UDP_DIR)

$(BUILD_DIR)/udp/client: $(UDP_DIR)/client.c | $(BUILD_DIR)/udp
	$(CC) $(UDP_DIR)/client.c -o $(BUILD_DIR)/udp/client -I $(UDP_DIR)

$(BUILD_DIR)/ip:
	mkdir -p $(BUILD_DIR)/ip

$(BUILD_DIR)/udp:
	mkdir -p $(BUILD_DIR)/udp

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean ip udp