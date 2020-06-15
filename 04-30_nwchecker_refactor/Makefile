objs = nwc_connection.o \
	   nwc_looper.o \
	   nwc_epoll_looper.o \
	   nwc_main.o \
	   nwc_client.o \
	   nwc_server.o \
	   nwc_configuration.o \
	   nwc_tcp_handler.o \
	   nwc_udp_handler.o \
	   nwc.o \
	   nwc_sock.o \
	   nwc_qos_protocol.o

DEBUG_FLAG = -g

all:$(objs)
	$(CC) $(LDFLAGS) $^ -o nwc

$(objs):%.o:%.c
	$(CC) $(DEBUG_FLAG) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm *.o nwc


