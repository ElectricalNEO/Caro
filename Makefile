
SOURCES = $(wildcard *.c)

caro: $(SOURCES)
	gcc $^ -o $@
