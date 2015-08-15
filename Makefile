#test_timer
#
# 3 cases to test the timer
#

CC := gcc
CFLAGS := -Wall -g
DIR := $(shell pwd)
LIBS := -ltfal -lrt 
LDFLAGS := -L$(DIR)
SRC := $(wildcard *.c)
BINAME := a

all:$(BINAME)

libtfal.a: 
	$(MAKE) -C libtfal
	cp ./libtfal/libtfal.a .

$(BINAME):libtfal.a
	$(CC) $(-CFLAGS) $(SRC) -o $(BINAME) $(LIBS) $(LDFLAGS)

clean:
	$(MAKE) -C libtfal $@
	@rm -f *.o *.a