#ifndef __CHUM_H
#define __CHUM_H

#include "opencoap.h"


typedef struct {
   coap_resource_desc_t desc;
} chum_vars_t;

//=========================== prototypes ======================================

void chum_init(void);


#endif
