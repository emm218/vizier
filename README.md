# Vizier

minimalist html documentation tool for C code.

## Project Structure

Vizier produces 1 documentation page per invocation. If a `.c` file is provided
and it includes a header of the same name then symbols defined in the header
will be documented on the same documentation page. for example

```shell
vizier doc paging.c
```

would document items in both `paging.c` and `paging.h`, assuming that `paging.c`
included `paging.h`.

I've considered adding more manual control over grouping but I'm unsure of some
of the design details and dont need it anyway for my own use case, so for now im
choosing not to implement it.

## Usage

Vizier is meant to be integrated with `make` through a snippet such as the one
below (extracted from my project [eos](https://github.com/emm218/eos):

```make

DOC:=docs

.vizier/%.d: .depend/%.d
    sed 's/^/\.vizier/;s/\.o/\.html/' $<

include $(patsubst %.c, .vizier/%.d, $(filter %.c, $(SRC)))

docs: $(patsubst %.c, $(DOC)/%.html, $(SRC))

%.html:
    vizier doc $(CFLAGS) $< > %@

```

this assumes that `CC` is set to your C compiler, `SRC` contains a list of
source files you wish to be documented, and `DOC` containers the folder to put
the final docs in.
