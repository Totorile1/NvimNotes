CC := gcc

VERSION := $(shell git describe --tags --always --dirty)

CFLAGS := -Wall -Wextra -O2 \
		  -DVERSION=\"$(VERSION)\" \
	      $(shell pkg-config --cflags libcjson ncurses)

LDFLAGS := $(shell pkg-config --libs libcjson ncurses)

SRC := src/main.c src/ui.c src/utils.c src/notes.c
TARGET := notewrapper

.PHONY: all run clean

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@timeout 5s ./$(TARGET); \
	status=$$?; \
	if [ $$status -eq 124 ]; then \
		echo "Program ran for 5 seconds without crashing ✅"; \
		exit 0; \
	elif [ $$status -ne 0 ]; then \
		echo "Program crashed ❌"; \
		exit 1; \
	else \
		echo "Program exited cleanly ✅"; \
		exit 0; \
	fi
