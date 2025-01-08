CC = gcc
BUILD_DIR = build

all: $(BUILD_DIR)/server $(BUILD_DIR)/client

$(BUILD_DIR)/server: server.c | $(BUILD_DIR)
	$(CC) server.c -o $(BUILD_DIR)/server -I .

$(BUILD_DIR)/client: client.c | $(BUILD_DIR)
	$(CC) client.c -o $(BUILD_DIR)/client -I .

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean