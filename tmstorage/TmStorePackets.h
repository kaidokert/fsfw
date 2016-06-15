/*
 * TmStorePackets.h
 *
 *  Created on: 11.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_
#define FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_

#include <framework/serialize/SerialFixedArrayListAdapter.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

class ServiceSubservice: public SerialLinkedListAdapter<SerializeIF> {
public:
	SerializeElement<uint8_t> service;
	SerialFixedArrayListAdapter<uint8_t, 16> subservices;
	LinkedElement<SerializeIF> linkedSubservices;
	ServiceSubservice() :
			SerialLinkedListAdapter<SerializeIF>(&service), linkedSubservices(
					&subservices) {
		service.setNext(&linkedSubservices);
	}
};

class ChangeSelectionDefinition: public SerialLinkedListAdapter<SerializeIF> {
public:
	SerializeElement<uint16_t> apid;
	SerialFixedArrayListAdapter<ServiceSubservice, 16> serviceList;
	LinkedElement<SerializeIF> linkedServiceList;
	ChangeSelectionDefinition() :
			SerialLinkedListAdapter<SerializeIF>(&apid), linkedServiceList(
					&serviceList) {
		apid.setNext(&linkedServiceList);
	}
};

class TmPacketInformation: public SerialLinkedListAdapter<SerializeIF> {
public:
	TmPacketInformation(TmPacketMinimal* packet) :
			SerialLinkedListAdapter<SerializeIF>(&apid) {
		setAllNext();
		setContent(packet);
	}
	TmPacketInformation() :
			SerialLinkedListAdapter<SerializeIF>(&apid), apid(
					SpacePacketBase::LIMIT_APID), sourceSequenceCount(0), serviceType(
					0), serviceSubtype(0), subCounter(0) {
		setAllNext();
	}
	void reset() {
		apid = SpacePacketBase::LIMIT_APID;
		sourceSequenceCount = 0;
		serviceType = 0;
		serviceSubtype = 0;
		subCounter = 0;
	}
	void setContent(TmPacketMinimal* packet) {
		apid = packet->getAPID();
		sourceSequenceCount = packet->getPacketSequenceCount();
		serviceType = packet->getService();
		serviceSubtype = packet->getSubService();
		subCounter = packet->getPacketSubcounter();
	}
	void setContent(TmPacketInformation* content) {
		apid.entry = content->apid.entry;
		sourceSequenceCount.entry = content->sourceSequenceCount.entry;
		serviceType.entry = content->serviceType.entry;
		serviceSubtype.entry = content->serviceSubtype.entry;
		subCounter.entry = content->subCounter.entry;
	}
	bool isValid() {
		return (apid < SpacePacketBase::LIMIT_APID) ? true : false;
	}
private:
	SerializeElement<uint16_t> apid;
	SerializeElement<uint16_t> sourceSequenceCount;
	SerializeElement<uint8_t> serviceType;
	SerializeElement<uint8_t> serviceSubtype;
	SerializeElement<uint8_t> subCounter;
	void setAllNext() {
		apid.setNext(&sourceSequenceCount);
		sourceSequenceCount.setNext(&serviceType);
		serviceType.setNext(&serviceSubtype);
		serviceSubtype.setNext(&subCounter);
	}
};

#endif /* FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_ */
