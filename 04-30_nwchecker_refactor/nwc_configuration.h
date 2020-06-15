#ifndef _NWC_CONFIGURATION_H_
#define _NWC_CONFIGURATION_H_

struct nwc_configuration
{
    const char *ip;
    unsigned short port;
    int mode; // The app mode
    int prototype; // The protocol type
    int count_secs; // The test time, The unit is seconds.
    int bandwidth; // The user-defined bandwidth. 
};

int nwc_configuration_parse(struct nwc_configuration *config, int argc, char *argv[]);

#endif//_NWC_CONFIGURATION_H_

