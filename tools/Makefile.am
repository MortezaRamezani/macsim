AUTOMAKE_OPTIONS = foreign
SUBDIRS =

all:
	./trace_reader/knobgen.pl
	make -C trace_reader

clean:
	rm -f ./trace_reader/all_knobs.*
	make clean -C trace_reader

