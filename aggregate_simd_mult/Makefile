
.PHONY: all clean doc distclean 

include system.inc

CXX=g++
CPPFLAGS=-Idist/include/
#CPPFLAGS+=-DPREFETCH
#CPPFLAGS+=-DOUTPUT_AGGREGATE
#CPPFLAGS+=-DOUTPUT_WRITE_NT
#CPPFLAGS+=-DOUTPUT_WRITE_NORMAL
CPPFLAGS+=-mavx2
CPPFLAGS+=-DOUTPUT_ASSEMBLE
CPPFLAGS+=-DLOGGROUPS=$(logGroups)
#CPPFLAGS+=-DDEBUG #-DDEBUG2
CXXFLAGS=$(SYSFLAGS)
#CXXFLAGS+=-g -O0 #-Wall
CXXFLAGS+=-O3 -std=c++11
LDFLAGS=-Ldist/lib/
LDLIBS= -lpthread -lbz2

FILES = parser.o table.o ProcessorMap.o Barrier.o loader.o partitioner.o affinitizer.o aggregator.o

all: dist aggregate

clean:
	rm -f *.o
	rm -f aggregate

distclean: clean
	rm -rf dist

aggregate: $(FILES) main.o

dist:
	./pre-init.sh
