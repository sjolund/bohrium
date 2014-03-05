ROOT=..
include $(ROOT)/platform.mk
LIB=libbh.$(LIB_SUFFIX)
CC=g++
WARNOPTS=-Wall
LCFLAGS= $(WARNOPTS) $(ARCH_OPTS) -g -O2 -fPIC -std=gnu++0x -I$(ROOT)/include
LDFLAGS= $(ARCH_OPTS) -shared -ldl $(EXTRA_LIBS) -pthread -Wl,$(LD_LIB_COMMAND),$(LIB)
SRC=$(ALL_CPP_FILES)
OBJ=$(SRC:.cpp=.o) $(ROOT)/core/iniparser/libiniparser.a
HEADER=$(ALL_H_FILES) $(ALL_HPP_FILES) $(ROOT)/include/*

all: $(LIB)

%.o: %.c $(ROOT)/include/*
	$(CC) -c $< -o $@ $(LCFLAGS) $(CFLAGS)

$(LIB): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -rf *.o *~ $(LIB)

install: all
	cp $(LIB) $(INSTALLDIR)
