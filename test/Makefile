
INSTALL_DIR:=../install

CXX?=g++
CXXFLAGS:=-std=c++11 -Wall -DNDEBUG -O2 -I$(INSTALL_DIR)/include
LINKFLAGS:=-pthread -L$(INSTALL_DIR)/lib -lnet

BUILD_DIR:=build
LIB:=$(INSTALL_DIR)/lib/libnet.a

SRCS:=$(wildcard *.cpp)
TARGETS:=$(patsubst %.cpp, $(BUILD_DIR)/%, $(SRCS))

.PHONY: all
all: $(BUILD_DIR) $(TARGETS)

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR)/%: %.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LINKFLAGS)
