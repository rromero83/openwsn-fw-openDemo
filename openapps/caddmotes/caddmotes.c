#include "opendefs.h"
#include "caddmotes.h"
#include "opencoap.h"
#include "opentimers.h"
#include "openqueue.h"
#include "packetfunctions.h"
#include "scheduler.h"
#include "idmanager.h"
#include "openserial.h"
#include "IEEE802154E.h"


//=========================== defines =========================================

/// inter-packet period (in ms)
#define CADDMOTESPERIOD  30000

const uint8_t caddmotes_path0[] = "caddmotes";

//=========================== variables =======================================

caddmotes_vars_t caddmotes_vars;

//=========================== prototypes ======================================

owerror_t caddmotes_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options);
void caddmotes_timer_cb(opentimer_id_t id);
void caddmotes_task_cb(void);
void caddmotes_sendDone(OpenQueueEntry_t* msg, owerror_t error);

//=========================== public ==========================================

void caddmotes_init() {
	//No inicialitzar si és DAGroot
	if (idmanager_getIsDAGroot() == TRUE)
			return;

	// prepare the resource descriptor for the /caddmotes path
	caddmotes_vars.desc.path0len = sizeof(caddmotes_path0) - 1;
	caddmotes_vars.desc.path0val = (uint8_t*) (&caddmotes_path0);
	caddmotes_vars.desc.path1len = 0;
	caddmotes_vars.desc.path1val = NULL;
	caddmotes_vars.desc.componentID = COMPONENT_CADDMOTES;
	caddmotes_vars.desc.callbackRx = &caddmotes_receive;
	caddmotes_vars.desc.callbackSendDone = &caddmotes_sendDone;

	opencoap_register(&caddmotes_vars.desc);
	caddmotes_vars.timerId = opentimers_start(CADDMOTESPERIOD, TIMER_PERIODIC,
			TIME_MS, caddmotes_timer_cb);
}

//=========================== private =========================================

owerror_t caddmotes_receive(OpenQueueEntry_t* msg, coap_header_iht* coap_header,
		coap_option_iht* coap_options) {
	owerror_t outcome;
	switch (coap_header->Code) {

	case COAP_CODE_REQ_PUT:

		//Stop timer
		if (msg->payload[0] == '1') {
			opentimers_stop(caddmotes_vars.timerId);
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

//timer fired, but we don't want to execute task in ISR mode
//instead, push task to scheduler with COAP priority, and let scheduler take care of it
void caddmotes_timer_cb(opentimer_id_t id) {
	scheduler_push_task(caddmotes_task_cb, TASKPRIO_COAP);
}

void caddmotes_task_cb() {
	OpenQueueEntry_t* pkt;
	owerror_t outcome;

	// don't run if not synch
	if (ieee154e_isSynch() == FALSE)
		return;

	// create a CoAP RD packet
	pkt = openqueue_getFreePacketBuffer(COMPONENT_CADDMOTES);
	if (pkt == NULL) {
		openserial_printError(COMPONENT_CADDMOTES, ERR_NO_FREE_PACKET_BUFFER,
				(errorparameter_t) 0, (errorparameter_t) 0);
		openqueue_freePacketBuffer(pkt);
		return;
	}
	// take ownership over that packet
	pkt->creator = COMPONENT_CADDMOTES;
	pkt->owner = COMPONENT_CADDMOTES;

	packetfunctions_reserveHeaderSize(pkt, 1);
	pkt->payload[0] = COAP_PAYLOAD_MARKER;

	// content-type option
	packetfunctions_reserveHeaderSize(pkt, 2);
	pkt->payload[0] = (COAP_OPTION_NUM_CONTENTFORMAT - COAP_OPTION_NUM_URIPATH)
			<< 4 | 1;
	pkt->payload[1] = COAP_MEDTYPE_APPOCTETSTREAM;
	// location-path option
	packetfunctions_reserveHeaderSize(pkt, sizeof(caddmotes_path0) - 1);
	memcpy(&pkt->payload[0], caddmotes_path0, sizeof(caddmotes_path0) - 1);
	packetfunctions_reserveHeaderSize(pkt, 1);
	pkt->payload[0] = ((COAP_OPTION_NUM_URIPATH) << 4)
			| (sizeof(caddmotes_path0) - 1);

	// metadata
	pkt->l4_destination_port = WKP_UDP_COAP;
	pkt->l3_destinationAdd.type = ADDR_128B;
	memcpy(&pkt->l3_destinationAdd.addr_128b[0], &ipAddr_ringmaster, 16);
	// send
	outcome = opencoap_send(pkt, COAP_TYPE_NON, COAP_CODE_REQ_PUT, 1,
			&caddmotes_vars.desc);

	// avoid overflowing the queue if fails
	if (outcome == E_FAIL) {
		openqueue_freePacketBuffer(pkt);
	}

	return;
}

void caddmotes_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
	openqueue_freePacketBuffer(msg);
}
