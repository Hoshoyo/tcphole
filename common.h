#pragma once
#include <netinet/ip.h>

typedef enum {
    INFO_NONE,
    INFO_SELF_PUBLIC,
    INFO_REMOTE_PUBLIC,
} Info_Type;

typedef struct {
    Info_Type      type;
    int            index;

    // Remote addr
    struct in_addr remote_ip;
    in_port_t      remote_port;

    // Local addr
    struct in_addr local_ip;
    in_port_t      local_port;
} ClientInfo;