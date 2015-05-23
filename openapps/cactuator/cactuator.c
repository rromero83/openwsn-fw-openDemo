#include "opendefs.h"
#include "cactuator.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "idmanager.h"


#include "gpio.h"
#include <headers/hw_memmap.h>

//=========================== defines =========================================

const uint8_t cactuator_path0[] = "cactuator";

//Defines PB5
#define BSP_GPIO_BASE            GPIO_B_BASE
#define BSP_GPIO_PB5             GPIO_PIN_5

//=========================== variables =======================================

cactuator_vars_t cactuator_vars;

//=========================== prototypes ======================================

owerror_t cactuator_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options);

void cactuator_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void cactuator_init(void) {
	//No inicialitzar si és DAGroot
	if (idmanager_getIsDAGroot() == TRUE)
		return;

	// register to OpenCoAP module
	cactuator_vars.desc.path0len = sizeof(cactuator_path0) - 1;
	cactuator_vars.desc.path0val = (uint8_t*) (&cactuator_path0);
	cactuator_vars.desc.path1len = 0;
	cactuator_vars.desc.path1val = NULL;
	cactuator_vars.desc.componentID = COMPONENT_CACTUATOR;
	cactuator_vars.desc.callbackRx = &cactuator_receive;
	cactuator_vars.desc.callbackSendDone = &cactuator_sendDone;
	opencoap_register(&cactuator_vars.desc);

	//Inicialitzar PB5
	gpio_pb5_init();

}

//=========================== private =========================================

owerror_t cactuator_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options) {
	owerror_t outcome;

	switch (coap_header->Code) {
	case COAP_CODE_REQ_GET:
		// reset packet payload
		msg->payload = &(msg->packet[127]);
		msg->length = 0;

		// add CoAP payload
		packetfunctions_reserveHeaderSize(msg, 2);
		msg->payload[0] = COAP_PAYLOAD_MARKER;

		//Obtenir estat GPIO
		if (gpio_pb5_isOn() == 1) {
			msg->payload[1] = '1';
		} else {
			msg->payload[1] = '0';
		}

		// set the CoAP header
		coap_header->Code = COAP_CODE_RESP_CONTENT;

		outcome = E_SUCCESS;
		break;

	case COAP_CODE_REQ_PUT:
		//Canviar estat GPIO
		if (msg->payload[0] == '0') {
			gpio_pb5_off();
		} else if (msg->payload[0] == '1') {
			gpio_pb5_on();
		}

		// reset packet payload
		msg->payload = &(msg->packet[127]);
		msg->length = 0;

		// set the CoAP header
		coap_header->Code = COAP_CODE_RESP_CHANGED;

		outcome = E_SUCCESS;
		break;

	default:
		outcome = E_FAIL;
		break;
	}

	return outcome;
}

void cactuator_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
	openqueue_freePacketBuffer(msg);
}

//Funció per inicialitzar GPIO
void gpio_pb5_init() {
	GPIOPinTypeGPIOOutput(BSP_GPIO_BASE, BSP_GPIO_PB5);
	GPIOPinWrite(BSP_GPIO_BASE, BSP_GPIO_PB5, 0);
}

//Funció que obté l'estat del GPIO
uint8_t gpio_pb5_isOn() {
	uint32_t ui32Toggle = GPIOPinRead(BSP_GPIO_BASE, BSP_GPIO_PB5);
	return (uint8_t) (ui32Toggle & BSP_GPIO_PB5) >> 5;
}

//Funció per desactivar el GPIO
void gpio_pb5_off() {
	GPIOPinWrite(BSP_GPIO_BASE, BSP_GPIO_PB5, 0);
}

//Funció per activar el GPIO
void gpio_pb5_on() {
	GPIOPinWrite(BSP_GPIO_BASE, BSP_GPIO_PB5, BSP_GPIO_PB5);
}
