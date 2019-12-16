CFLAGS = -Wall -g
all: h265bs_parse_stream h265bs_parse_file

h265bs_parse_stream: h265bs_parse_stream.c
	gcc ${CFLAGS} -o $@ $^ -pthread

h265bs_parse_file: h265bs_parse_file.c
	gcc ${CFLAGS} -o $@ $^

.PHONY: clean distclean

clean:
	rm -rf h265bs_parse_stream h265bs_parse_file

distclean: clean
