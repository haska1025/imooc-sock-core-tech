objs = nwc_connection.o \
	   nwc_epoll_looper.o \
	   nwc_main.o \
	   nwc_tcp_listener.o \
	   nwc_tcp_handler.o \
	   nwc.o \
	   nwc_sock.o

DEBUG_FLAG = -g

all:$(objs)
	$(CC) $(LDFLAGS) $^ -o nwc

$(objs):%.o:%.c
	$(CC) $(DEBUG_FLAG) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm *.o


