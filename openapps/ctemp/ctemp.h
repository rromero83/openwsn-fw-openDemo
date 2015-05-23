#ifndef __CTEMP_H
#define __CTEMP_H

#include "opencoap.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} ctemp_vars_t;

//=========================== prototypes ======================================

void ctemp_init(void);


#endif
