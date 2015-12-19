/*
 * VoiceStreamStats.h
 *
 *  Created on: Nov 25, 2015
 *      Author: mike
 */

#ifndef LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_
#define LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_

#include "XBee.h"

class VoiceStreamStats {

	private:
		XBeeAddress64 senderAddress;
		double throughput;
		unsigned long totalPacketsRecieved;
		uint8_t expectedFrameId;
		uint8_t packetLoss;
		uint8_t receivedFrame;
		uint8_t intervalStartFrame;
		double timeStamp;
		uint8_t totalPacketsSent;
		double voiceQuality;
		uint8_t duplicateFrame;

	public:
		VoiceStreamStats(const XBeeAddress64& senderAddress);
		const XBeeAddress64& getSenderAddress() const;
		double getThroughput() const;
		void calculateThroughput();
		void updateVoiceLoss(const uint8_t * dataPtr);
		void printRouteEnd() const;
		void startStream() const;
		uint8_t getPacketLoss() const;
};

#endif /* LIBRARIES_VOICESTATISTICS_VOICESTREAMSTATS_H_ */