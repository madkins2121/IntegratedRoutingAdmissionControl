/*
 * HeartbeatMessage.h
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_
#define LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_

#include "Arduino.h"
#include "XBee.h"
#include <math.h>

using namespace std;

class HeartbeatMessage {

	private:

		float dataRate;
		XBeeAddress64 senderAddress;
		XBeeAddress64 downStreamNeighbor;

	public:
		HeartbeatMessage();
		HeartbeatMessage(const float dataRate);
		HeartbeatMessage(const float dataRate, const XBeeAddress64& senderAddress);
		void generateBeatMessage(uint8_t payload[]);
		void transcribeHeartbeatPacket(const Rx64Response& response);
		void setDataRate(float dataRate);
		float getDataRate() const;
		const XBeeAddress64& getSenderAddress() const;
		const XBeeAddress64& getDownStreamNeighbor() const;
		void setDownStreamNeighbor(const XBeeAddress64& downStreamNeighbor);
		void print();
};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_HEARTBEATMESSAGE_H_ */
