/*
 * HeartBeatProtocol.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: mike
 */
#include "HeartbeatProtocol.h"

#define SINK_ADDRESS_1 0x0013A200
#define SINK_ADDRESS_2 0x40B519CC
#define DEBUG true

const uint8_t HEARTBEAT_PAYLOAD_SIZE = 24;
const float MAX_FLT = 9999.0;
const float EPISLON = 0.001;

HeartbeatProtocol::HeartbeatProtocol() {
	seqNum = 0;
	routeFlag = false;
	qualityOfPath = 0;
	dataRate = 0;
	neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;
	buildSaturationTable();
}

HeartbeatProtocol::HeartbeatProtocol(const XBeeAddress64& broadcastAddress, const XBeeAddress64& myAddress,
		const XBeeAddress64& sinkAdress, XBee& xbee) {
	this->seqNum = 0;
	this->xbee = xbee;
	this->myAddress = myAddress;
	this->sinkAddress = sinkAdress;
	this->routeFlag = false;
	this->broadcastAddress = broadcastAddress;
	this->qualityOfPath = 0;
	this->dataRate = 0;
	this->neighborhoodCapacity = MAX_FLT;
	timeoutLength = 0;

	if (myAddress.equals(sinkAddress)) {
		routeFlag = true;
	}
	buildSaturationTable();
}

void HeartbeatProtocol::broadcastHeartBeat() {

	if (DEBUG) {
		printNeighborHoodTable();
	}

	HeartbeatMessage message = HeartbeatMessage(sinkAddress, seqNum, dataRate, qualityOfPath, neighborhoodCapacity,
			routeFlag);

	uint8_t payload[HEARTBEAT_PAYLOAD_SIZE];

	message.generateBeatMessage(payload);

	Tx64Request tx = Tx64Request(broadcastAddress, payload, sizeof(payload));

	xbee.send(tx);

	seqNum++;

}

void HeartbeatProtocol::reCalculateNeighborhoodCapacity() {
	float neighborhoodRate = 0;
	uint8_t neighborhoodSize = 0;
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		neighborhoodRate += (*neighborhoodTable.at(i)).getDataRate();
		if (abs((*neighborhoodTable.at(i)).getDataRate() - 0) > EPISLON) {
			neighborhoodSize++;
		}
	}

	//Don't forget to include myself
	neighborhoodRate += dataRate;

	if (abs(dataRate - 0) > EPISLON) {
		neighborhoodSize++;
	}

	if (neighborhoodSize == 1 || neighborhoodSize == 0) {
		neighborhoodCapacity = MAX_FLT;
	} else if (neighborhoodSize > 1) {
		float saturationRate = satT[neighborhoodSize - 2].getRate();
		neighborhoodCapacity = saturationRate - neighborhoodRate;
	}

}

void HeartbeatProtocol::receiveHeartBeat(const Rx64Response& response, bool ignoreHeartBeatFlag) {
	HeartbeatMessage message;

	message.transcribeHeartbeatPacket(response);

	if (!myAddress.equals(sinkAddress)) {
		routeFlag = message.isRouteFlag();
	}

	updateNeighborHoodTable(message);
	reCalculateNeighborhoodCapacity();

	if (!myAddress.equals(sinkAddress)) {
		if ((*nextHop).equals(Neighbor())) {
			noNeighborcalculatePathQualityNextHop();
		} else if ((*nextHop).getAddress().equals(message.getSenderAddress())) {
			withNeighborcalculatePathQualityNextHop();
		}
	}
}

void HeartbeatProtocol::updateNeighborHoodTable(const HeartbeatMessage& heartbeatMessage) {

	bool found = false;

	for (int i = 0; i < neighborhoodTable.size(); i++) {
		if ((*neighborhoodTable.at(i)).getAddress().equals(heartbeatMessage.getSenderAddress())) {
			updateNeighbor((*neighborhoodTable.at(i)), heartbeatMessage);

			if ((*neighborhoodTable.at(i)).equals((*nextHop))) {
				SerialUSB.println("Updated Next Hop with info");
				nextHop = (*neighborhoodTable.at(i));
			}

			found = true;
			break;
		}
	}

	if (!found) {
		//Sets timestamp when constructor is called.
		Neighbor neighbor = Neighbor(heartbeatMessage.getSenderAddress(), heartbeatMessage.getDataRate(),
				heartbeatMessage.getSeqNum(), heartbeatMessage.getQualityOfPath(),
				heartbeatMessage.getNeighborhoodCapacity(), heartbeatMessage.isRouteFlag(),
				heartbeatMessage.getSinkAddress(), heartbeatMessage.getRelativeDistance(), heartbeatMessage.getRssi(),
				timeoutLength);
		neighborhoodTable.push_back(&neighbor);
	}

}

void HeartbeatProtocol::updateNeighbor(Neighbor& neighbor, const HeartbeatMessage& heartbeatMessage) {
	neighbor.setDataRate(heartbeatMessage.getDataRate());
	neighbor.setSeqNum(heartbeatMessage.getSeqNum());
	neighbor.setQualityOfPath(heartbeatMessage.getQualityOfPath());
	neighbor.setNeighborhoodCapacity(heartbeatMessage.getNeighborhoodCapacity());
	neighbor.setRouteFlag(heartbeatMessage.isRouteFlag());
	neighbor.setSinkAddress(heartbeatMessage.getSinkAddress());
	neighbor.setRelativeDistance(heartbeatMessage.getRelativeDistance());
	neighbor.setRssi(heartbeatMessage.getRssi());
	neighbor.updateTimeStamp();
}

void HeartbeatProtocol::purgeNeighborhoodTable() {

	if (!neighborhoodTable.empty()) {
		for (vector<Neighbor*>::iterator it = neighborhoodTable.begin(); it != neighborhoodTable.end();) {
			if ((*it)->timerExpired()) {
				SerialUSB.print("Neighbor: ");
				(*it)->getAddress().printAddressASCII(&SerialUSB);
				SerialUSB.println(" timer has expired and is purged.");
				delete *it;
				it = neighborhoodTable.erase(it);
			} else {
				++it;
			}

		}
	}
}

void HeartbeatProtocol::printNeighborHoodTable() {
	SerialUSB.println();
	SerialUSB.print("MyAddress: ");
	myAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", DataRate: ");
	SerialUSB.print(dataRate);
	SerialUSB.print(", SeqNum: ");
	SerialUSB.print(seqNum);
	SerialUSB.print(", QualityOfPath: ");
	SerialUSB.print(qualityOfPath);
	SerialUSB.print(", NeighborhoodCapacity: ");
	SerialUSB.print(neighborhoodCapacity);
	SerialUSB.print(", RouteFlag: ");
	SerialUSB.print(routeFlag);
	SerialUSB.print(", SinkAddress: ");
	sinkAddress.printAddressASCII(&SerialUSB);
	SerialUSB.print(", NextHopAddress: ");
	(*nextHop).getAddress().printAddressASCII(&SerialUSB);
	SerialUSB.println();

	for (int i = 0; i < neighborhoodTable.size(); i++) {

		SerialUSB.print("NeighborAddress: ");
		(*neighborhoodTable.at(i)).getAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", DataRate: ");
		SerialUSB.print((*neighborhoodTable.at(i)).getDataRate());
		SerialUSB.print(", SeqNum: ");
		SerialUSB.print((*neighborhoodTable.at(i)).getSeqNum());
		SerialUSB.print(", QualityOfPath: ");
		SerialUSB.print((*neighborhoodTable.at(i)).getQualityOfPath());
		SerialUSB.print(", NeighborhoodCapacity: ");
		SerialUSB.print((*neighborhoodTable.at(i)).getNeighborhoodCapacity());
		SerialUSB.print(", RouteFlag: ");
		SerialUSB.print((*neighborhoodTable.at(i)).isRouteFlag());
		SerialUSB.print(", SinkAddress: ");
		(*neighborhoodTable.at(i)).getSinkAddress().printAddressASCII(&SerialUSB);
		SerialUSB.print(", RSSI: ");
		SerialUSB.print((*neighborhoodTable.at(i)).getRssi());
		SerialUSB.print(", RelativeDistance: ");
		SerialUSB.println((*neighborhoodTable.at(i)).getRelativeDistance(), 12);

	}
	SerialUSB.println();
}

void HeartbeatProtocol::noNeighborcalculatePathQualityNextHop() {

	//Add 1 to include myself
	uint8_t neighborHoodSize = neighborhoodTable.size() + 1;
	uint8_t qop = UINT8_MAX;
	Neighbor neighbor;

	SerialUSB.println("I need a nextHop!");
	/*If no neighbor is currently selected:
	 * - Pick neighbor with smallest quality of path
	 * - If quality of path is the same, pick with shorter relative distance
	 */
	for (int i = 0; i < neighborhoodTable.size(); i++) {

		if ((*neighborhoodTable.at(i)).isRouteFlag()) {
			uint8_t path = neighborHoodSize + (*neighborhoodTable.at(i)).getQualityOfPath();

			if (path < qop) {
				qop = path;
				neighbor = (*neighborhoodTable.at(i));

			} else if (qop == path) {
				double relativeDistanceCurrent = neighbor.getRelativeDistance();
				double relativeDistanceNew = (*neighborhoodTable.at(i)).getRelativeDistance();
				if (relativeDistanceCurrent > relativeDistanceNew) {
					neighbor = (*neighborhoodTable.at(i));
				}
			}
		}
	}

	//Make sure route exists
	if (qop != UINT8_MAX) {
		qualityOfPath = qop;
		nextHop = neighbor;
		routeFlag = true;

	} else {
		//reset path if path does not exist
		qualityOfPath = 0;
		nextHop = Neighbor();
		routeFlag = false;
	}

}

void HeartbeatProtocol::withNeighborcalculatePathQualityNextHop() {

	//Add 1 to include myself

	//Neighbor is already selected.  Should I make some adjustments?

	unsigned long timeStamp = (*nextHop).getTimeStamp();
	unsigned long previousTimeStamp = (*nextHop).getPreviousTimeStamp();

	SerialUSB.print("TimeStamp: ");
	SerialUSB.println(timeStamp);
	SerialUSB.print("PreviousTimeStamp: ");
	SerialUSB.println(previousTimeStamp);

	unsigned long diff = timeStamp - previousTimeStamp;
	SerialUSB.println("Should I adjust my nextHop neighbor?");
	SerialUSB.print("Difference from last timeStamp: ");
	SerialUSB.println(diff);

}

void HeartbeatProtocol::buildSaturationTable() {
	satT[0] = Saturation(2, 120.90);
	satT[1] = Saturation(3, 153.39);
	satT[2] = Saturation(4, 151.2);
	satT[3] = Saturation(5, 154.45);
	satT[4] = Saturation(6, 111.42);

}

float HeartbeatProtocol::getLocalCapacity() const {

	float localCapacity = MAX_FLT;
	for (int i = 0; i < neighborhoodTable.size(); i++) {
		float neighborCapacity = (*neighborhoodTable.at(i)).getNeighborhoodCapacity();
		if (neighborCapacity < localCapacity) {
			localCapacity = neighborCapacity;
		}

	}

	if (neighborhoodCapacity < localCapacity) {
		localCapacity = neighborhoodCapacity;
	}

	SerialUSB.print("Local Capacity: ");
	SerialUSB.println(localCapacity);
	return localCapacity;
}

bool HeartbeatProtocol::isRouteFlag() const {
	return routeFlag;
}

const Neighbor* HeartbeatProtocol::getNextHop() const {
	return nextHop;
}

float HeartbeatProtocol::getDataRate() const {
	return dataRate;
}

void HeartbeatProtocol::setDataRate(float dataRate) {
	this->dataRate = dataRate;
}

const XBeeAddress64& HeartbeatProtocol::getBroadcastAddress() const {
	return broadcastAddress;
}

void HeartbeatProtocol::setBroadcastAddress(const XBeeAddress64& broadcastAddress) {
	this->broadcastAddress = broadcastAddress;
}

unsigned long HeartbeatProtocol::getTimeoutLength() const {
	return timeoutLength;
}

void HeartbeatProtocol::setTimeoutLength(unsigned long timeoutLength) {
	this->timeoutLength = timeoutLength;
}

const XBee& HeartbeatProtocol::getXbee() const {
	return xbee;
}

void HeartbeatProtocol::setXbee(const XBee& xbee) {
	this->xbee = xbee;
}

