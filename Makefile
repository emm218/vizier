CC=clang

CFLAGS+=-Wall -Wextra -Werror -fpic
LDFLAGS+=-fuse-ld=lld
LDLIBS+=-lclang

SRC:=$(wildcard *.c)

OBJ:=$(SRC:.c=.o) MurmurHash3.o

debug: CFLAGS+=-g
debug: LDFLAGS+=-g
debug: vizier

release: CFLAGS+=-O2
release: vizier

vizier: $(OBJ)

.depend/%.d: %.c
	@mkdir -p .depend 
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $^ -MF $@

include $(patsubst %.c, .depend/%.d, $(SRC))

clean:
	rm -f *.o doctool

.PHONY: release clean
