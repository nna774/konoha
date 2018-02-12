include Makefile.common
TMPDIR := tmp
SRCDIR := src
DEBUGDIR := debug
DEBUGFILES := memwatch.log

all: $(TARGET)
$(TARGET): build
	$(CP) $(SRCDIR)/$(TARGET) .

build:
	$(MAKE) -C $(SRCDIR) build

debug: clean
	$(MAKE) -C $(DEBUGDIR) all
	$(MAKE) -C $(SRCDIR) debug
	$(CP) $(SRCDIR)/$(TARGET) .

clean: clean_without_target clean_debug
	$(RM) $(TARGET) $(DEBUGFILES)

clean_src:
	$(MAKE) -C $(SRCDIR) clean

clean_debug:
	$(MAKE) -C $(DEBUGDIR) clean

clean_without_target: clean_src
	$(RM) *.o *.s ./$(TMPDIR)/*

test: $(TARGET) self_driver.s
	mkdir -p "$(TMPDIR)"
	CC=$(CC) ./test.sh

self_driver.s:
	./$(TARGET) self_driver.c -o self_driver.s

.PHONY: clean clean_src test self_driver.s
