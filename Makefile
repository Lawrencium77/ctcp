CC = gcc
BUILD_DIR = build
IP_DIR = src/ip

# Add debug flags if DEBUG is set
ifdef DEBUG
    CFLAGS += -g -DDEBUG
endif

all: ip

ip: $(BUILD_DIR)/ip/server $(BUILD_DIR)/ip/client

$(BUILD_DIR)/ip/server: $(IP_DIR)/server.c | $(BUILD_DIR)/ip
	$(CC) $(CFLAGS) $(IP_DIR)/server.c -o $(BUILD_DIR)/ip/server -I $(IP_DIR)

$(BUILD_DIR)/ip/client: $(IP_DIR)/client.c | $(BUILD_DIR)/ip
	$(CC) $(CFLAGS) $(IP_DIR)/client.c -o $(BUILD_DIR)/ip/client -I $(IP_DIR)

$(BUILD_DIR)/ip:
	mkdir -p $(BUILD_DIR)/ip

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean ip