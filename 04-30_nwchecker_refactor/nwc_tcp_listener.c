#include "nwc_tcp_listener.h"

struct nwc_tcp_listener_open(struct nwc_tcp_listener *listener, const char *ip, uint16_t port)
{
    int rc = 0;
    int sock_fd = -1;
    struct sockaddr_in server_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        printf("Create socket failed! errno(%d)\n", errno);
        return -1;
    }

    int flags = fcntl(sock_fd, F_GETFL);
    flags |= O_NONBLOCK; 
    fcntl(sock_fd, F_SETFL, flags);

    int enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        printf("setsockopt(SO_REUSEADDR) failed errno(%d)", errno);
        close(sock_fd);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc == -1){
        printf("Bind server failed! errno(%d)\n", errno);
        close(sock_fd);
        return -1;
    }

    rc = listen(sock_fd, 3);
    if (rc == -1){
        printf("Server listen failed! errno(%d)\n", errno);
        close(sock_fd);
        return -1;
    }

    listener->parent.handle_input = nwc_tcp_listener_handle_input;
    listener->parent.handle_output = NULL;
    listener->parent.fd = sock_fd;


    listener->parent.h_handle = nwc_looper_add_handler(listner->parent.looper, EV_READ);
}
void nwc_tcp_listener_handle_input(struct nwc_io_handler *handler)
{
}

