
SUBDIRS:= message http rtsp rpc
#SRCS:= 

ifeq ("$(USE_OPENSSL)", "1")
SUBDIRS += ssl
endif

ifeq ("$(USE_MYSQL)", "1")
SUBDIRS += db
endif

include $(TOPDIR)/common.mk
