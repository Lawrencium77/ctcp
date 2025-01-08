CC = gcc
BUILD_DIR = build
SRC_DIR = src

all: $(BUILD_DIR)/server $(BUILD_DIR)/client

$(BUILD_DIR)/server: $(SRC_DIR)/server.c | $(BUILD_DIR)
	$(CC) $(SRC_DIR)/server.c -o $(BUILD_DIR)/server -I $(SRC_DIR)

$(BUILD_DIR)/client: $(SRC_DIR)/client.c | $(BUILD_DIR)
	$(CC) $(SRC_DIR)/client.c -o $(BUILD_DIR)/client -I $(SRC_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean