CFLAGS = -c -Wall -Wextra
LDFLAGS =
PORT ?= 7331

EXE_NAME = server

SRC_DIR = src
OUT_DIR = build/linux
OBJ_DIR = $(OUT_DIR)/obj
OUT_EXE = $(OUT_DIR)/$(EXE_NAME)

SRC_FILES = $(shell find $(SRC_DIR)/ -type f -name '*.c')
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))


.PHONY: build
build: $(OUT_EXE)

.PHONY: run
run: build
	./$(OUT_EXE) $(PORT)

.PHONY: clean
clean:
	rm -rf "$(OUT_DIR)" "$(OBJ_DIR)"

$(OUT_EXE): $(OBJ_FILES)
	@mkdir -p "$(OUT_DIR)"
	@echo Linking "$(OUT_EXE)" ...
	@$(CC) $(LDFLAGS) -o "$(OUT_EXE)" $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p "$(@D)"
	@echo Compiling "$<" ...
	@$(CC) $(CFLAGS) -o $@ $<
