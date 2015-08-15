#TFAL - short for "timer for app layer"
#

CC := gcc
CFLAGS := -Wall -g
TFAL_SRC := $(wildcard *.c)
OBJS := $(TFAL_SRC:.c=.o)
BIN := libtfal.a

all:$(BIN)

clean:
	@rm -f *.o *.a

$(OBJS): %.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN):$(OBJS)
	ar -r $@ $^