#ifndef _NWCHECER_H_
#define _NWCHECER_H_


struct nwc_args
{
    const char *ip;
    unsigned short port;
    int count;
    int no_close;
    int interval;  // The unit is ms
    int sent_pkgs; // The sent packages of every slot.
    int message_size; // The send message size. 
    int echo_mode; // 0:immediate, 1: random time, 2: no response, 3: server not recv
};

int nwc_client(struct nwc_args *na);
int nwc_server(struct nwc_args *na);

char * alloc_buffer(int alloc_size);
void free_buffer(char *buff);


#endif//_NWCHECER_H_

