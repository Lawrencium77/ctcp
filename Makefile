CC = gcc
BUILD_DIR = build
SRC_DIR = src
IP_DIR = src/ip
UDP_DIR = src/udp

# Add debug flags if DEBUG is set
ifdef DEBUG
    CFLAGS += -g -DDEBUG
endif

all: ip udp

ip: $(BUILD_DIR)/ip/server $(BUILD_DIR)/ip/client
udp: $(BUILD_DIR)/udp/server $(BUILD_DIR)/udp/client

$(BUILD_DIR)/ip/server: $(IP_DIR)/server.c | $(BUILD_DIR)/ip
	$(CC) $(CFLAGS) $(IP_DIR)/server.c -o $(BUILD_DIR)/ip/server -I $(SRC_DIR) -I $(IP_DIR)

$(BUILD_DIR)/ip/client: $(IP_DIR)/client.c | $(BUILD_DIR)/ip
	$(CC) $(CFLAGS) $(IP_DIR)/client.c -o $(BUILD_DIR)/ip/client -I $(SRC_DIR) -I $(IP_DIR)

$(BUILD_DIR)/ip:
	mkdir -p $(BUILD_DIR)/ip

$(BUILD_DIR)/udp/server: $(UDP_DIR)/server.c | $(BUILD_DIR)/udp
	$(CC) $(CFLAGS) $(UDP_DIR)/server.c $(UDP_DIR)/checksum.c -o $(BUILD_DIR)/udp/server -I $(SRC_DIR) -I $(UDP_DIR)

$(BUILD_DIR)/udp/client: $(UDP_DIR)/client.c | $(BUILD_DIR)/udp
	$(CC) $(CFLAGS) $(UDP_DIR)/client.c $(UDP_DIR)/checksum.c -o $(BUILD_DIR)/udp/client -I $(SRC_DIR) -I $(UDP_DIR)

$(BUILD_DIR)/udp:
	mkdir -p $(BUILD_DIR)/udp

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean ip