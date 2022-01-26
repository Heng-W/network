
INSTALL_DIR:=../../install

CXX?=g++
CXXFLAGS:=-std=c++11 -Wall -DNDEBUG -O2 -I$(INSTALL_DIR)/include
LINKFLAGS:=-pthread -L$(INSTALL_DIR)/lib -lnet

LIB:=$(INSTALL_DIR)/lib/libnet.a

all: server.out client.out

server.out: server.cpp query.msg.h $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LINKFLAGS)

client.out: client.cpp query.msg.h $(SRCS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LINKFLAGS)
