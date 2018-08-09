CURRENT_PWD = $(shell pwd)

HANDLE_PATH_INCLUDE:= $(HOME_PATH_INCLUDE)/handler/


CURRENT_PATH_INCLUDE:=$(CURRENT_PWD)

CFLAGS  := -lpthread
CFLAGS2 := -I$(HANDLE_PATH_INCLUDE) -I$(CURRENT_PATH_INCLUDE) -Wformat-extra-args -fpermissive

LD_FLAGS:= -shared -fpic

CC := g++
#LD := ld
LD := $(CC) -shared 

OBJS_TARGET_LIBS := WKRtsp.o WKRtspCmd.o WKRtspCtrl.o WKRtspRequest.o WKRtspResponseDecoder.o WKSessionContext.o WKNetUtils.o \
					WKMediaProcessor.o  WKStreamCtrl.o WKStream.o WKTSPackageQueue.o WKUDP.o WKURLParser.o WKTSPackage.o
OBJS_TARGET:=  rtspClient.o

HANDLE_LIBS:= -L$(HOME_PATH_LIBS) -lHandle
RTSPCLIENT_LIBS:= -L$(HOME_PATH_LIBS) -lRtsp $(HANDLE_LIBS)


all: libs rtspclient

libs:libRtsp.so

%.o : %.S
	$(CC) -g -c $^
	
%.o : %.c
	$(CC) -g -fPIC  $(CFLAGS2) -c $^
%.o : %.cpp
	$(CC) -g -fPIC  $(CFLAGS2) -c $^

rtspclient:$(OBJS_TARGET)
	$(CC) -o $@  $^  $(CFLAGS) $(CFLAGS2) $(RTSPCLIENT_LIBS)

libRtsp.so:$(OBJS_TARGET_LIBS)
	 $(LD) $(LD_FLAGS) -o   $@ $^  $(CFLAGS2) $(HANDLE_LIBS) $(CFLAGS)
	 echo "install " $@ "to" $(HOME_PATH_LIBS)
	 mv $@ $(HOME_PATH_LIBS)
clean:
	rm *.o wmessagetest -rf