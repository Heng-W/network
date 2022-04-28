
file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(net OBJECT ${SRCS})


set(INSTALL_HEAD_FILES tcp_client.h tcp_client_thread.h tcp_connection.h callbacks.h buffer.h inet_address.h endian.h)

install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net)
