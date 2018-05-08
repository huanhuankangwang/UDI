HOME_PATH := $(shell pwd)
HOME_PATH_INCLUDE:= $(HOME_PATH)/include

HOME_PATH_LIBS:=$(HOME_PATH)/lib

CFLAGS  := -lpthread
CFLAGS2 := -I$(HOME_PATH_INCLUDE) -Wformat-extra-args
CC:= g++
LD:=ld

OBJS_TARGET:= utils.o threads.o WKMessage.o WKMessageQueue.o WkHandler.o WKLooper.o Looper.o WKMessageTest.o

all:wmessagetest

%.o : %.S
	$(CC) -g -c $^
	
%.o : %.c
	$(CC) -g -fPIC  $(CFLAGS2) -c $^
%.o : %.cpp
	$(CC) -g -fPIC  $(CFLAGS2) -c $^

wmessagetest:$(OBJS_TARGET)
	$(CC) -o $@  $^  $(CFLAGS) $(CFLAGS2)
	
clean:
	rm *.o wmessagetest -rf