# Makefile for dirwatch.c
# Usage:
#   make            # build release (default)
#   make debug      # build with debug flags + ASan/UBSan (optional)
#   make run DIR=.  # run with a directory (default DIR=.)
#   make clean
#   make install PREFIX=/usr/local
#   make uninstall PREFIX=/usr/local

# ---- Toolchain ----
CC      ?= cc

# ---- Files ----
TARGET  := dirwatch
SRC     := dirwatch.c
OBJ     := $(SRC:.c=.o)
DEP     := $(OBJ:.o=.d)

# ---- Flags ----
# Common warnings and standard
WARN    := -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes
STD     := -std=c11
DEFS    := 
INCS    := 
OPT     := -O2
DBG     := -g3

# Enable dependency generation
DEPFLAGS := -MMD -MP

# Default CFLAGS/LDFLAGS (release)
CFLAGS  ?= $(STD) $(WARN) $(OPT) $(INCS) $(DEFS) $(DEPFLAGS)
LDFLAGS ?=
LDLIBS  ?=

# Directory to run against
DIR ?= .


# ---- Targets ----
.PHONY: all release debug clean run install uninstall

all: release

release: $(TARGET)

debug: CFLAGS := $(STD) $(WARN) $(DBG) $(INCS) $(DEFS) $(DEPFLAGS) -fsanitize=address,undefined
debug: LDFLAGS := $(LDFLAGS) -fsanitize=address,undefined
debug: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET) "$(DIR)"

install: $(TARGET)
	install -d "$(PREFIX)/bin"
	install -m 0755 "$(TARGET)" "$(PREFIX)/bin/$(TARGET)"

uninstall:
	rm -f "$(PREFIX)/bin/$(TARGET)"

clean:
	rm -f $(TARGET) $(OBJ) $(DEP)

# Include auto-generated dependencies
-include $(DEP)
