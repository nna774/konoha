TOP_DIR := ..
SRC_DIR := .

include $(TOP_DIR)/Makefile.common

CFLAGS := -Wall -Wextra

build: $(TARGET)

$(TARGET): $(TARGET).o

clean:
	$(RM) $(TARGET) *.o

.PHONY: clean
