# what flags you want to pass to the C compiler & linker

XFLAGS   = -DLONG_COUNTERS
WARN_FLAGS  = -Wunused-function -Wreturn-type -Wpointer-arith
DEBUG_FLAGS = -DNO_DEBUG

AM_LDFLAGS = -lz

#
# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = trace_reader

trace_reader_CPPFLAGS = -O3 -std=c++0x -funroll-loops -Werror $(DEBUG_FLAGS) $(WARN_FLAGS) $(XFLAGS) -I../../src
trace_reader_SOURCES = \
	main.cc \
	all_knobs.cc \
	../../src/knob.cc

