objs = nwc_connection.o \
	   nwc_epoll_looper.o \
	   nwc_looper.o \
	   nwc_main.o

all:$(objs)
	$(CC) $(LDFLAGS) $^ -o nwc

$(objs):%.o:%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm *.o


