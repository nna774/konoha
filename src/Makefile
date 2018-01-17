TOP_DIR := ..
SRC_DIR := .

include $(TOP_DIR)/Makefile.common
SRCS := konoha.c
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

CFLAGS := -Wall -Wextra

build: $(TARGET)

$(TARGET): $(OBJS)

%.o: %.c
	$(CC) -c -MMD -MP $<

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
