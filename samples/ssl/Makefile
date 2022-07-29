
INSTALL_DIR:=../../install

CXX?=g++
CXXFLAGS:=-std=c++11 -Wall -DNDEBUG -O2 -I$(INSTALL_DIR)/include

OPENSSL_LIB?=/usr/local/openssl/lib
LINKFLAGS:=-pthread -L$(INSTALL_DIR)/lib -L$(OPENSSL_LIB) -lnet -lcrypto -lssl -ldl

LIB:=$(INSTALL_DIR)/lib/libnet.a

all: server.out client.out

server.out: server.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LINKFLAGS)

client.out: client.cpp $(SRCS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LINKFLAGS)
