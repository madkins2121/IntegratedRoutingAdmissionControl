/*
 * VoiceStreamsManager.h
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_
#define LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_

#include <vector>
#include "VoiceStreamStats.h"
#include "XBee.h"

using namespace std;

class VoiceStreamManager {

	private:
		vector<VoiceStreamStats> streams;
		XBee xbee;
		uint8_t payloadSize;
		double timeDifference;
		bool setTimeDifference = true;

	public:
		VoiceStreamManager();
		VoiceStreamManager(const uint8_t payloadSize);
		VoiceStreamManager(XBee& xbee, const uint8_t payloadSize);

		//void calcuateThroughput(const XBeeAddress64& packetSource);
		void updateVoiceLoss(const XBeeAddress64& packetSource, const XBeeAddress64& previousHop,
				const uint8_t * dataPtr);
		void updateStreamsIntermediateNode(const XBeeAddress64& packetSource, const XBeeAddress64& previousHop);
		void getStreamPreviousHop(const XBeeAddress64& packetSource, XBeeAddress64& previousHop);

		void removeStream(const XBeeAddress64& packetSource);
		void sendPathPacket();
		void calculateThroughput();
		uint8_t getPayloadSize() const;
};

#endif /* LIBRARIES_VOICESTATISTICS_VOICESTREAMSMANAGER_H_ */
