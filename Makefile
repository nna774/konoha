all: konoha
.PHONY: clean test

TMPDIR := tmp
RM := rm -f
CFLAGS=-Wall -Wextra

konoha: konoha.o

clean:
	$(RM) konoha *.o $(TMPDIR)/*

test: konoha
	mkdir -p $(TMPDIR)
	./test.sh
