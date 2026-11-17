CC     := gcc
CFLAGS := -O2 -g -Wall -Wextra -MMD -MP -Iinclude $(EXTRA_CFLAGS)
LDLIBS := -lssl -lcrypto

PREFIX ?= $(HOME)/.local
TARGET := kernelwatch
SRCS   := $(wildcard src/*.c) 
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(strip $(SRCS)),)
$(error no sources found in src/)
endif
endif
OBJS   := $(SRCS:.c=.o)

.PHONY: all install clean check

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

check:
	checkpatch.pl --no-tree -f src/*.c include/*.h

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)

-include $(OBJS:.o=.d)
