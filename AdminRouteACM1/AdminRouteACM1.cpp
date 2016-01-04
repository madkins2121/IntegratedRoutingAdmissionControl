// Do not remove the include below
#include "AdminRouteACM1.h"

#define STATUS_LED 13
#define ERROR_LED 12
#define DEBUG false
#define VOICE_DATA_INTERVAL 3000
#define SENDER false
#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define HEARTBEAT_ADDRESS_1 0x00000000
#define HEARTBEAT_ADDRESS_2 0x0000FFFF
#define IGNORE_HEARTBEAT false
//#define HEARTBEAT_ADDRESS_1 0x0013A200
//#define HEARTBEAT_ADDRESS_2 0x40B31805

XBee xbee = XBee();
HeartbeatProtocol * heartbeatProtocol;
VoicePacketSender * voicePacketSender;

ThreadController controller = ThreadController();
Thread heartbeat = Thread();
Thread sendData = Thread();
Thread responseThread = Thread();
Thread pathLoss = Thread();

XBeeAddress64 heartBeatAddress = XBeeAddress64(HEARTBEAT_ADDRESS_1, HEARTBEAT_ADDRESS_2);
XBeeAddress64 sinkAddress = XBeeAddress64(SINK_ADDRESS_1, SINK_ADDRESS_2);
XBeeAddress64 myAddress;

void setup() {
	arduinoSetup();

	xbee.getMyAddress(myAddress, DEBUG);
	heartbeatProtocol = new HeartbeatProtocol(heartBeatAddress, myAddress, sinkAddress, xbee);
	voicePacketSender = new VoicePacketSender(xbee, heartbeatProtocol, &pathLoss, myAddress, sinkAddress, 2, 0);
	setupThreads();

	digitalWrite(13, LOW);
}

void loop() {
	controller.run();
}

void arduinoSetup() {
	pinMode(STATUS_LED, OUTPUT);
	pinMode(ERROR_LED, OUTPUT);

	randomSeed(analogRead(0));

	Serial.begin(111111);
	SerialUSB.begin(111111);

	xbee.setSerial(Serial);

	digitalWrite(13, HIGH);
	unsigned long start = millis();

	clearBuffer();

	while (millis() - start < 8000) {

	}

}

void sendVoicePacket() {
	voicePacketSender->generateVoicePacket();
}

void broadcastHeartbeat() {
	heartbeatProtocol->broadcastHeartBeat();

	/*if (heartbeatProtocol.isRouteFlag()) {
	 voicePacketSender.setMyNextHop(heartbeatProtocol.getNextHopAddress());
	 if (SENDER)
	 sendData.enabled = true;
	 } else {
	 //No Next Hop
	 voicePacketSender.setMyNextHop(XBeeAddress64());
	 }*/

}

void sendPathPacket() {
	voicePacketSender->sendPathPacket();
}

void clearBuffer() {
	while (Serial.available())
		Serial.read();
}

void listenForResponses() {
	//checkTimers();

	if (xbee.readPacketNoTimeout(DEBUG)) {
		if (xbee.getResponse().getApiId() == RX_64_RESPONSE) {
			Rx64Response response;
			xbee.getResponse().getRx64Response(response);
			uint8_t* data = response.getData();

			char control[5];

			for (int i = 0; i < 5; i++) {
				control[i] = data[i];
			}

			if (!strcmp(control, "BEAT")) {
				//routing data
				heartbeatProtocol->receiveHeartBeat(response, IGNORE_HEARTBEAT);
			} else if (!strcmp(control, "DATA")) {
				//voice data
				voicePacketSender->handleDataPacket(response);
			} else if (!strcmp(control, "PATH")) {
				//path loss packet
				voicePacketSender->handlePathPacket(response);
			} else if (!strcmp(control, "RSTR")) {
				voicePacketSender->handleStreamRestart(response);
			}
			/*else if(!strcmp(control, "ASM_")) {
			 /*When I receive the neighborhood rate of a neighbor I update my neighborhood rate.
			 admissionController.updateNeighborRatesCalculateMyNeighborhoodRate(response);
			 } else if(!strcmp(control, "INIT")) {
			 handleInitPacket(response);
			 } else if(!strcmp(control, "REDJ")) {
			 handleREDJPacket(response);
			 } else if(!strcmp(control, "GRNT")) {
			 handleGRANTPacket(response);
			 }*/
		}
	}
}

void setupThreads() {

	responseThread.ThreadName = "Packet Listener";
	responseThread.enabled = true;
	responseThread.setInterval(1);
	responseThread.onRun(listenForResponses);

	heartbeat.ThreadName = "Broadcast Heartbeat";
	heartbeat.enabled = true;
	heartbeat.setInterval(3000 + random(100));
	heartbeat.onRun(broadcastHeartbeat);

	pathLoss.ThreadName = "Send Path Loss";
	pathLoss.enabled = false;
	pathLoss.setInterval(3000 + random(100));
	pathLoss.onRun(sendPathPacket);

	sendData.ThreadName = "Send Voice Data";
	if (SENDER) {
		sendData.enabled = true;
	} else {
		sendData.enabled = false;
	}
	sendData.setInterval(VOICE_DATA_INTERVAL);
	sendData.onRun(sendVoicePacket);

	controller.add(&heartbeat);
	controller.add(&responseThread);
	controller.add(&sendData);
	controller.add(&pathLoss);
}
