all: konoha
.PHONY: clean test

TMPDIR := tmp
RM := rm -f

konoha: konoha.o

clean:
	$(RM) konoha *.o $(TMPDIR)/*

test: konoha
	./test.sh
