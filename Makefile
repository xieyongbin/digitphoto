CROSS_COMPILE ?= arm-linux-

AS		= $(CROSS_COMPILE)as
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
LD		= $(CROSS_COMPILE)ld

CFLAGS 	:= -Wall -O0 -g -DMEMWATCH -DMEMWATCH_STDIO
CFLAGS 	+= -I $(shell pwd)/include -I $(shell pwd)/common 
CFLAGS  += -I /home/xieyb/master/stage_dir/target/freetype/jz2440_4.3.2/usr/local/include/
CFLAGS  += -I /home/xieyb/master/stage_dir/target/tslib/jz2440_4.3.2/usr/local/include/

LDFLAGS := -lm -lpthread -lrt
LDFLAGS += -L /home/xieyb/master/stage_dir/target/freetype/jz2440_4.3.2/usr/local/lib/ -lfreetype
LDFLAGS += -L /home/xieyb/master/stage_dir/target/tslib/jz2440_4.3.2/usr/local/lib/ -lts


export AS CC CPP AR LD
export CFLAGS LDFLAGS

TOP_OBJS	= main.o		\
              debug.o		\
              libthreadpro.o \
              display/		\
              draw/			\
              page/			\
              pic/			\
              encode/		\
              fonts/		\
              input/		\
              file/			\
              log/			\
              kfifo/		\
              mem/			\

TOP_DIR := $(shell pwd)

export TOP_OBJS
export TOP_DIR

TARGET = digitphoto

all:
	make -C ./ -f $(TOP_DIR)/Makefile.build
	$(CC) $(CFLAGS) -o $(TARGET) ./built-in.o $(LDFLAGS) 

clean:
	-rm -f $(shell find -name "*.o")
	-rm -f $(TARGET)

distclean:
	-rm -f $(shell find -name "*.o")
	-rm -f $(shell find -name "*.d")
	-rm -f $(TARGET)



