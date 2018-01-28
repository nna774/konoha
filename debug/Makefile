LIBDEBUG := libdebug.a
all: $(LIBDEBUG)

include ../Makefile.common

SRCS := memwatch.c
OBJS := $(SRCS:%.c=%.o)
CFLAGS := -w

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(LIBDEBUG): $(OBJS)
	ar rcs $@ $<

clean:
	$(RM) $(LIBDEBUG) $(OBJS)

.PHONY: clean
