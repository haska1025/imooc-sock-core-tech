#ifndef _NWCHECER_H_
#define _NWCHECER_H_


struct nwc_args
{
    const char *ip;
    unsigned short port;
    int count;
    int no_close;
};

int nwc_client(struct nwc_args *na);
int nwc_server(struct nwc_args *na);

#endif//_NWCHECER_H_

