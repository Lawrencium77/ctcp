BUILD_DIR = build

clean:
	rm -rf $(BUILD_DIR)

lint: lint-format lint-tidy

lint-format:
	@find src -name '*.c' -o -name '*.h' | xargs clang-format -i

lint-tidy:
	@find src -name '*.c' | xargs clang-tidy -p build/

.PHONY: clean lint lint-format lint-tidy