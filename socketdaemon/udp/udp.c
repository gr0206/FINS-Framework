/**
 * 	UDP test.c
 *
 *  Created on: Jun 30, 2010
 *      Author: Abdallah Abdallah
 */

//Kevin added some debugging code in this file to print out the received control frames and send back a response
//The control frames were never updated to the new types

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <finstypes.h>
#include <queueModule.h>
#include "udp.h"

#define DUMMYA 123
#define DUMMYB 456
#define DUMMYC 789

struct udp_statistics udpStat;
extern sem_t UDP_to_Switch_Qsem;
extern finsQueue UDP_to_Switch_Queue;

extern sem_t Switch_to_UDP_Qsem;
extern finsQueue Switch_to_UDP_Queue;

void sendToSwitch(struct finsFrame *ff) {

	sem_wait(&UDP_to_Switch_Qsem);
	write_queue(ff, UDP_to_Switch_Queue);
	sem_post(&UDP_to_Switch_Qsem);

}

void udp_get_FF() {

	int dummy_a, dummy_b, dummy_c;	///KEVINS CODE THIS IS A TEST

	struct finsFrame *ff;
	do {
		sem_wait(&Switch_to_UDP_Qsem);
		ff = read_queue(Switch_to_UDP_Queue);
		sem_post(&Switch_to_UDP_Qsem);
	} while (ff == NULL);

	udpStat.totalRecieved++;
	PRINT_DEBUG("UDP Total %d", udpStat.totalRecieved);
	if (ff->dataOrCtrl == CONTROL) 
	{
		// send to something to deal with FCF
		PRINT_DEBUG("UDP: CONTROL HANDLER !");

		//dummy = (int)(ff->ctrlFrame.paramterValue);

//CHANGEME	if(ff->ctrlFrame.paramterID == DUMMYA)
		{
//CHANGEME		dummy_a = (int)(ff->ctrlFrame.paramterValue);
			PRINT_DEBUG("UDP: Dummy parameter A has been set to %d",dummy_a);

		}
//CHANGEME		else if(ff->ctrlFrame.paramterID == DUMMYB)
		{
//CHANGEME		dummy_b = (int)(ff->ctrlFrame.paramterValue);
			PRINT_DEBUG("UDP: Dummy parameter B has been set to %d",dummy_b);
		}
//CHANGEME		else if(ff->ctrlFrame.paramterID == DUMMYC)
		{
//CHANGEME			dummy_c = (int)(ff->ctrlFrame.paramterValue);
			PRINT_DEBUG("UDP: Dummy parameter C has been set to %d",dummy_c);
		}

		//DEBUG Statements
		PRINT_DEBUG("UDP: dataOrCtrl parameter has been set to %d",(int)(ff->dataOrCtrl));				///KEVINS CODE THIS IS A TEST
		PRINT_DEBUG("UDP: destinationID parameter has been set to %d",(int)(ff->destinationID.id));		///KEVINS CODE THIS IS A TEST
		PRINT_DEBUG("UDP: opcode parameter has been set to %d",ff->ctrlFrame.opcode);					///KEVINS CODE THIS IS A TEST
		PRINT_DEBUG("UDP: senderID parameter has been set to %d",(int)(ff->ctrlFrame.senderID));		///KEVINS CODE THIS IS A TEST
//CHANGEME		PRINT_DEBUG("UDP: parameterID parameter has been set to %d",ff->ctrlFrame.paramterID);			///KEVINS CODE THIS IS A TEST

	//	PRINT_DEBUG("UDP: serialNum has been set to %d",ff->ctrlFrame.serialNum);

		//CONSTRUCTION OF A WRITE_CONFIRMATION FRAME
		//|| Data/Control | Destination_IDs_List | SenderID | Write_parameter_Confirmation_Code | Serial_Number ||
		struct finsFrame *ff_confirmation = (struct finsFrame *) malloc(sizeof(struct finsFrame));		///KEVINS CODE THIS IS A TEST
		ff_confirmation->dataOrCtrl = ff->dataOrCtrl;													///KEVINS CODE THIS IS A TEST
		ff_confirmation->destinationID.id = ff->ctrlFrame.senderID;										///KEVINS CODE THIS IS A TEST
		ff_confirmation->ctrlFrame.senderID = ff->destinationID.id;										///KEVINS CODE THIS IS A TEST
//CHANGEME		ff_confirmation->ctrlFrame.opcode = WRITECONF;						// hard coded				///KEVINS CODE THIS IS A TEST
		ff_confirmation->ctrlFrame.serialNum = ff->ctrlFrame.serialNum; 	//same as the Write REquest ///KEVINS CODE THIS IS A TEST

		//SEND TO QUEUE
		sem_wait(&UDP_to_Switch_Qsem);																	///KEVINS CODE THIS IS A TEST
		write_queue(ff_confirmation, UDP_to_Switch_Queue);												///KEVINS CODE THIS IS A TEST
		sem_post(&UDP_to_Switch_Qsem);																	///KEVINS CODE THIS IS A TEST
		PRINT_DEBUG("UDP: sent data ");																	///KEVINS CODE THIS IS A TEST


	}
	if ((ff->dataOrCtrl == DATA) && ((ff->dataFrame).directionFlag == UP)) {
		udp_in(ff);
		PRINT_DEBUG();
	}
	if ((ff->dataOrCtrl == DATA) && ((ff->dataFrame).directionFlag == DOWN)) {
		udp_out(ff);
		PRINT_DEBUG();
	}

}

void udp_init() {
	PRINT_DEBUG("UDP Started");
	while (1) {

		udp_get_FF();
		PRINT_DEBUG();
		//	free(pff);


	}

}
