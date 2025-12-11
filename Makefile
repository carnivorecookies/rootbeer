DESTDIR ?= /
PREFIX ?= /usr/local

SRCS = main.c service.c socket.c
SRCS := $(addprefix src/,$(SRCS))

objdir = .obj
objs := $(patsubst src/%.c,$(objdir)/%.o,$(SRCS))

deps = pam
ifeq ($(shell uname -s),Linux)
deps += libbsd-overlay
endif

CFLAGS += -std=c17 -Wall -Wextra -Wpedantic -MMD -MP
CFLAGS +=  $(shell pkgconf --cflags $(deps))
LDFLAGS += $(shell pkgconf --libs $(deps))

all: beer

beer: $(objs)
	$(CC) -o $@ $(objs) $(LDFLAGS)
	chmod 755 beer

install:
	# TODO: copy readme to doc
	install -Dm755 beer $(DESTDIR)$(PREFIX)/bin
	install -Dm644 LICENSE $(DESTDIR)$(PREFIX)/share/doc/beer/copyright

dist: clean
	# TODO: version, readme
	tar czf beer.tar.gz src LICENSE Makefile

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/beer

check:
	pkgconf $(deps)

clean:
	rm -rf $(objdir)
	rm -f beer

$(objdir)/%.o: src/%.c | $(objdir)
	$(CC) -o $@ -c $< $(CFLAGS) -MF $(patsubst %.o,%.d,$@)

$(objdir):
	mkdir -p $@

.PHONY: all install uninstall check clean