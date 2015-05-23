#include "opendefs.h"
#include "chum.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "sht21.h"
#include "idmanager.h"

//=========================== defines =========================================

const uint8_t chum_path0[] = "hum";

//=========================== variables =======================================

chum_vars_t chum_vars;

//=========================== prototypes ======================================

owerror_t chum_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options);

void chum_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void chum_init(void) {
	//No inicialitzar si és DAGroot
	if (idmanager_getIsDAGroot() == TRUE)
		return;
	// register to OpenCoAP module
	chum_vars.desc.path0len = sizeof(chum_path0) - 1;
	chum_vars.desc.path0val = (uint8_t*) (&chum_path0);
	chum_vars.desc.path1len = 0;
	chum_vars.desc.path1val = NULL;
	chum_vars.desc.componentID = COMPONENT_CHUM;
	chum_vars.desc.callbackRx = &chum_receive;
	chum_vars.desc.callbackSendDone = &chum_sendDone;
	opencoap_register(&chum_vars.desc);

}

//=========================== private =========================================

owerror_t chum_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options) {
	owerror_t outcome;

	switch (coap_header->Code) {

	case COAP_CODE_REQ_GET:

		// reset packet payload
		msg->payload = &(msg->packet[127]);
		msg->length = 0;

		// add CoAP payload
		packetfunctions_reserveHeaderSize(msg, 3);
		msg->payload[0] = COAP_PAYLOAD_MARKER;
		uint16_t humSend = 0x0;
		//Verificar que el sensor SHT21 és present
		if (sht21_is_present() == true) {
			//Inicialitza comunicació amb sensor SHT21 a través del bus I2C
			sht21_init();
			//Llegir valor humitat del sensor SHT21
			humSend = sht21_read_humidity();
		}

		// return as big endian
		msg->payload[1] = (uint8_t) (humSend >> 8);
		msg->payload[2] = (uint8_t) (humSend & 0xff);

		// set the CoAP header
		coap_header->Code = COAP_CODE_RESP_CONTENT;

		outcome = E_SUCCESS;
		break;

	default:
		outcome = E_FAIL;
		break;
	}

	return outcome;
}

void chum_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
	openqueue_freePacketBuffer(msg);
}

