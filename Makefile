
CXX?=g++
CXXFLAGS:=-std=c++11 -Wall

PREFIX?=install
TARGET_NAME?=net


USE_OPENSSL?=0
OPENSSL_INCLUDE_DIR?=/usr/local/openssl/include

USE_MYSQL?=0
MYSQL_INCLUDE_DIR?=


TYPE?=Release
ifeq ($(TYPE), Debug)
CXXFLAGS+= -O -g
else ifeq ($(TYPE), Release)
CXXFLAGS+= -O2 -DNDEBUG
else
$(error TYPE invalid)
endif

ifneq ("$(MACRO)", "")
CXXFLAGS+=$(addprefix -D,$(MACRO))
endif


ifeq ($(TYPE), Debug)
TARGET_REAL_NAME:=lib$(TARGET_NAME).d.a
else
TARGET_REAL_NAME:=lib$(TARGET_NAME).a
endif


ifeq ($(TYPE), Debug)
OBJ_DIR_NAME?=obj.d
DEP_DIR_NAME?=dep.d
else
OBJ_DIR_NAME?=obj
DEP_DIR_NAME?=dep
endif


EXTRA_INCLUDE_DIR:= $(OPENSSL_INCLUDE_DIR)
EXTRA_LIB_DIR:= $(OPENSSL_LIB_DIR)

TOPDIR:=$(shell pwd)
SRC_TOPDIR:=src
OBJ_TOPDIR:=build/$(OBJ_DIR_NAME)
DEP_TOPDIR:=build/$(DEP_DIR_NAME)
TARGET_DIR:=bin
TARGET:=$(TARGET_DIR)/$(TARGET_REAL_NAME)

export CXX CXXFLAGS TOPDIR SRC_TOPDIR OBJ_TOPDIR DEP_TOPDIR EXTRA_INCLUDE_DIR INSTALL_DIR USE_OPENSSL USE_MYSQL


# Compile CXX project

.PHONY: all $(SRC_TOPDIR)
all: $(SRC_TOPDIR) $(TARGET_DIR)

$(SRC_TOPDIR):
	$(MAKE) -C $@
	@echo
	@echo "Check object files..."
	@$(MAKE) -f Makefile.ar TARGET=$(TARGET) OBJ_DIR_NAME="$(OBJ_DIR_NAME)"
	@echo


$(TARGET_DIR):
	mkdir -p $@
	

.PHONY: clean exec test install 

clean:
	rm -rf build/
	rm -f `find $(TARGET_DIR) -type f | egrep -v "(.sh$$|.bat$$)"`

test:
	$(MAKE) -C $@


INSTALL_DIR:=$(abspath $(PREFIX))
INSTALL_LIB_DIR:=$(INSTALL_DIR)/lib
INSTALL_LIB:=$(INSTALL_LIB_DIR)/$(TARGET_REAL_NAME)

install: $(INSTALL_LIB)
	$(MAKE) -C src $@
	@echo
	@echo "installed to: $(INSTALL_DIR)"
	@echo

ifeq ("$(wildcard $(INSTALL_LIB_DIR))", "")
$(INSTALL_LIB):$(TARGET) $(INSTALL_LIB_DIR)
else
$(INSTALL_LIB):$(TARGET)
endif
	cp $< $@

$(INSTALL_LIB_DIR):
	mkdir -p $@






