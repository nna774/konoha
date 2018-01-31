all: build

TOP_DIR := ..
SRC_DIR := .
DEBUG_DIR := ../debug

include $(TOP_DIR)/Makefile.common
SRCS := konoha.c ast.c utils.c use_list.c enum.c string.c use_enum.c tokenize.c
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

CFLAGS := -Wall -Wextra
-include $(DEPS)

build: $(TARGET)

$(TARGET): $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MP $<

debug: CFLAGS += -DDEBUG -g -I$(DEBUG_DIR) -L$(DEBUG_DIR) -ldebug
debug: clean
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean
