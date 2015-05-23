#include "opendefs.h"
#include "ctemp.h"
#include "opencoap.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "sht21.h"
#include "idmanager.h"


//=========================== defines =========================================

const uint8_t ctemp_path0[] = "temp";

//=========================== variables =======================================

ctemp_vars_t ctemp_vars;

//=========================== prototypes ======================================

owerror_t ctemp_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options);

void ctemp_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void ctemp_init(void) {
	//No inicialitzar si és DAGroot
	if (idmanager_getIsDAGroot() == TRUE)
		return;
	// register to OpenCoAP module
	ctemp_vars.desc.path0len = sizeof(ctemp_path0) - 1;
	ctemp_vars.desc.path0val = (uint8_t*) (&ctemp_path0);
	ctemp_vars.desc.path1len = 0;
	ctemp_vars.desc.path1val = NULL;
	ctemp_vars.desc.componentID = COMPONENT_CTEMP;
	ctemp_vars.desc.callbackRx = &ctemp_receive;
	ctemp_vars.desc.callbackSendDone = &ctemp_sendDone;
	opencoap_register(&ctemp_vars.desc);
}

//=========================== private =========================================

owerror_t ctemp_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
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
		uint16_t temp = 0x0;
		//Verificar que el sensor SHT21 és present
		if (sht21_is_present() == true) {
			//Inicialitza comunicació amb sensor SHT21 a través del bus I2C
			sht21_init();
			//Llegir valor temperatura del sensor SHT21
			temp = sht21_read_temperature();
		}

		// return as big endian
		msg->payload[1] = (uint8_t)(temp >> 8) ;
		msg->payload[2] = (uint8_t) (temp & 0xff);


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

void ctemp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
	openqueue_freePacketBuffer(msg);
}

