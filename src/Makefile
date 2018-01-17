all: build

TOP_DIR := ..
SRC_DIR := .

include $(TOP_DIR)/Makefile.common
SRCS := konoha.c ast.c utils.c use_list.c
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

CFLAGS := -Wall -Wextra
-include $(DEPS)

build: $(TARGET)

$(TARGET): $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MP $<

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
