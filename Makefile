include Makefile.common
TMPDIR := tmp
SRCDIR := src

all: $(TARGET)
$(TARGET): build
	$(CP) $(SRCDIR)/$(TARGET) .

build:
	$(MAKE) -C $(SRCDIR) build

clean: clean_src
	$(RM) $(TARGET) *.o ./$(TMPDIR)/*

clean_src:
	$(MAKE) -C $(SRCDIR) clean

test: $(TARGET)
	mkdir -p "$(TMPDIR)"
	CC=$(CC) ./test.sh

.PHONY: clean clean_src test
