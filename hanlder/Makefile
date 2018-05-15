CURRENT_PWD = $(shell pwd)

HANDLE_PATH_INCLUDE:= $(HOME_PATH_INCLUDE)/handler/

CFLAGS  := -lpthread
CFLAGS2 := -I$(HANDLE_PATH_INCLUDE) -Wformat-extra-args

LD_FLAGS:= -shared -fpic

CC := g++
LD := ld

OBJS_TARGET_LIBS := utils.o threads.o WKMessage.o WKMessageQueue.o WkHandler.o WKLooper.o Looper.o
OBJS_TARGET:=  $(OBJS_TARGET_LIBS) WKMessageTest.o

all:wmessagetest libs

libs:libHandle.so

%.o : %.S
	$(CC) -g -c $^
	
%.o : %.c
	$(CC) -g -fPIC  $(CFLAGS2) -c $^
%.o : %.cpp
	$(CC) -g -fPIC  $(CFLAGS2) -c $^

wmessagetest:$(OBJS_TARGET)
	$(CC) -o $@  $^  $(CFLAGS) $(CFLAGS2)

libHandle.so:$(OBJS_TARGET_LIBS)
	 $(LD) $(LD_FLAGS) -o   $@ $^
	 echo "install " $@ "to" $(HOME_PATH_LIBS)
	 mv $@ $(HOME_PATH_LIBS)
clean:
	rm *.o wmessagetest -rf