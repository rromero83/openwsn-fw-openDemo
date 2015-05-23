#ifndef __CACTUATOR_H
#define __CACTUATOR_H

#include "opencoap.h"

//=========================== variables =======================================

typedef struct {
   coap_resource_desc_t desc;
} cactuator_vars_t;

//=========================== prototypes ======================================

void cactuator_init(void);
void gpio_pb5_init(void);
void gpio_pb5_on(void);
void gpio_pb5_off(void);
uint8_t gpio_pb5_isOn(void);

#endif
