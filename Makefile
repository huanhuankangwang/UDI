HOME_PATH := $(shell pwd)

HOME_PATH_INCLUDE:= $(HOME_PATH)/include

HOME_PATH_LIBS:=$(HOME_PATH)/lib

export HOME_PATH HOME_PATH_INCLUDE HOME_PATH_LIBS


all:
	make -C hanlder
	make -C rtsp
	
clean:
	make clean -C hanlder
	make clean -C rtsp