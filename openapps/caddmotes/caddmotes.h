#ifndef __CADDMOTES_H
#define __CADDMOTES_H

#include "opencoap.h"

//=========================== typedef =========================================

typedef struct {
   coap_resource_desc_t desc;
   opentimer_id_t       timerId;
} caddmotes_vars_t;


//=========================== prototypes ======================================

void caddmotes_init(void);

#endif
