include Makefile.common
TMPDIR := tmp
SRCDIR := src

all: $(TARGET)
$(TARGET): build
	$(CP) $(SRCDIR)/$(TARGET) .

build:
	$(MAKE) -C $(SRCDIR) build

clean: clean_without_target
	$(RM) $(TARGET)

clean_src:
	$(MAKE) -C $(SRCDIR) clean

clean_without_target: clean_src
	$(RM) *.o ./$(TMPDIR)/*

test: $(TARGET)
	mkdir -p "$(TMPDIR)"
	CC=$(CC) ./test.sh

.PHONY: clean clean_src test
