/*
 * AdmissionControl.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mike
 */

#ifndef LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#define LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_
#include <XBee.h>
#include <HeartbeatProtocol.h>
#include <PotentialStream.h>
#include <VoiceStreamsManager.h>
#include <VoicePacketSender.h>
#include <Thread.h>
#include <map>

using namespace std;

class AdmissionControl {
	private:
		std::map<XBeeAddress64, PotentialStream> potentialStreams;
		XBee xbee;
		XBeeAddress64 myAddress;
		HeartbeatProtocol * heartbeatProtocol;
		VoiceStreamManager * voiceStreamManager;
		VoicePacketSender * voicePacketSender;
		unsigned long grantTimeoutLength;
		unsigned long rejcTimeoutLength;
		void addPotentialStream(PotentialStream& potentialStream, const float addDataRate);
		bool checkLocalCapacity(const PotentialStream& potentialStream);
	public:
		AdmissionControl();
		AdmissionControl(const XBeeAddress64& myAddress, const XBee& xbee, HeartbeatProtocol * heartbeatProtocol,
				VoiceStreamManager * voiceStreamManager, VoicePacketSender * voicePacketSender,
				const unsigned long grantTimeoutLength, const unsigned long rejcTimeoutLength);

		void sendInitPacket(const uint8_t codecSetting, const float dupSetting);
		void intializationSenderTimeout();
		void sendGRANTPacket(const XBeeAddress64 &senderAddress, const XBeeAddress64 &nextHop);
		void sendREDJPacket(const XBeeAddress64 &senderAddress);
		void handleInitPacket(const Rx64Response &response);
		void handleREDJPacket(Rx64Response &response);
		void handleGRANTPacket(const Rx64Response &response, bool& initThreadActive, bool& voiceThreadActive);
		void checkTimers();
		bool removePotentialStream(const XBeeAddress64& packetSource);
		void printPotentialStreams();

};

#endif /* LIBRARIES_HEARTBEATPROTOCOL_ADMISSIONCONTROL_H_ */
