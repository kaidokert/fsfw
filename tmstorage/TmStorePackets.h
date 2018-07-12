#ifndef FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_
#define FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_

#include <framework/serialize/SerialFixedArrayListAdapter.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>
#include <framework/timemanager/TimeStamperIF.h>
#include <framework/timemanager/CCSDSTime.h>
#include <framework/globalfunctions/timevalOperations.h>

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

class ApidSsc: public SerializeIF {
public:
	ApidSsc() :
			apid(SpacePacketBase::LIMIT_APID), ssc(0) {
	}
	ApidSsc(uint16_t apid, uint16_t ssc) :
			apid(apid), ssc(ssc) {
	}
	uint16_t apid;
	uint16_t ssc;
	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		ReturnValue_t result = SerializeAdapter<uint16_t>::serialize(&apid,
				buffer, size, max_size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		return SerializeAdapter<uint16_t>::serialize(&ssc, buffer, size,
				max_size, bigEndian);

	}

	uint32_t getSerializedSize() const {
		return sizeof(apid) + sizeof(ssc);
	}

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
	bool bigEndian) {
		ReturnValue_t result = SerializeAdapter<uint16_t>::deSerialize(&apid,
				buffer, size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		return SerializeAdapter<uint16_t>::deSerialize(&ssc, buffer, size,
				bigEndian);
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

class TmPacketInformation: public SerializeIF {
public:
	TmPacketInformation(TmPacketMinimal* packet){
		setContent(packet);
	}
	TmPacketInformation() :apid(
					SpacePacketBase::LIMIT_APID), sourceSequenceCount(0), serviceType(
					0), serviceSubtype(0), subCounter(0) {
	}
	void reset() {
		apid = SpacePacketBase::LIMIT_APID;
		sourceSequenceCount = 0;
		serviceType = 0;
		serviceSubtype = 0;
		subCounter = 0;
		memset(rawTimestamp, 0, sizeof(rawTimestamp));
	}
	void setContent(TmPacketMinimal* packet) {
		apid = packet->getAPID();
		sourceSequenceCount = packet->getPacketSequenceCount();
		serviceType = packet->getService();
		serviceSubtype = packet->getSubService();
		subCounter = packet->getPacketSubcounter();
		memset(rawTimestamp, 0, sizeof(rawTimestamp));
		const uint8_t* pField = NULL;
		uint32_t size = 0;
		ReturnValue_t result = packet->getPacketTimeRaw(&pField, &size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return;
		}
		if (*pField == CCSDSTime::P_FIELD_CDS_SHORT
				&& size <= TimeStamperIF::MISSION_TIMESTAMP_SIZE) {
			//Shortcut to avoid converting CDS back and forth.
			memcpy(rawTimestamp, pField, size);
			return;
		}
		timeval time = { 0, 0 };
		result = packet->getPacketTime(&time);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return;
		}

		CCSDSTime::CDS_short cdsFormat;
		result = CCSDSTime::convertToCcsds(&cdsFormat, &time);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return;
		}
		memcpy(rawTimestamp, &cdsFormat, sizeof(cdsFormat));
	}
	void setContent(TmPacketInformation* content) {
		apid = content->apid;
		sourceSequenceCount = content->sourceSequenceCount;
		serviceType = content->serviceType;
		serviceSubtype = content->serviceSubtype;
		subCounter = content->subCounter;
		memcpy(rawTimestamp, content->rawTimestamp, sizeof(rawTimestamp));
	}
	bool isValid() const {
		return (apid < SpacePacketBase::LIMIT_APID) ? true : false;
	}
	static void reset(TmPacketInformation* packet){
		packet->reset();
	}

	static bool isOlderThan(const TmPacketInformation* packet, const timeval* cmpTime){
		if(packet->isValid()){
			timeval packetTime = {0,0};
			uint32_t foundlen = 0;
			CCSDSTime::convertFromCcsds(&packetTime,&packet->rawTimestamp[0],&foundlen,sizeof(rawTimestamp));
			if(packetTime <= *cmpTime){
				return true;
			}
		}
		return false;
	}

	static bool isNewerThan(const TmPacketInformation* packet, const timeval* cmpTime){
		if(packet->isValid()){
			timeval packetTime = {0,0};
			uint32_t foundlen = 0;
			CCSDSTime::convertFromCcsds(&packetTime,&packet->rawTimestamp[0],&foundlen,sizeof(rawTimestamp));
			if(packetTime >= *cmpTime){
				return true;
			}
		}
		return false;
	}

	static bool isSmallerSSC(const TmPacketInformation* packet,const ApidSsc* compareSSC){
		if(packet->isValid()){
			if(packet->apid == compareSSC->apid){
				if(packet->sourceSequenceCount <= compareSSC->ssc){
					return true;
				}
			}
		}
		return false;
	}

	static bool isLargerSSC(const TmPacketInformation* packet,const ApidSsc* compareSSC){
		if(packet->isValid()){
			if(packet->apid == compareSSC->apid){
				if(packet->sourceSequenceCount >= compareSSC->ssc){
					return true;
				}
			}
		}
		return false;
	}

	uint16_t getApid() const{
		return apid;
	}

	uint16_t getSsc() const{
		return sourceSequenceCount;
	}

	uint8_t getServiceType() const{
		return serviceType;
	}

	uint8_t getServiceSubtype() const{
		return serviceSubtype;
	}

	uint8_t getSubCounter() const{
		return subCounter;
	}

	timeval getTime() const {
		timeval packetTime = {0,0};
		uint32_t foundlen = 0;
		CCSDSTime::convertFromCcsds(&packetTime,&this->rawTimestamp[0],&foundlen,sizeof(rawTimestamp));
		return packetTime;
	}

	bool operator==(const TmPacketInformation& other) {
		//TODO Does not compare Raw Timestamp
		return ((apid == other.getApid())
				&& (sourceSequenceCount == other.getSsc())
				&& (serviceType == other.getServiceType()) && (serviceSubtype =
						other.getServiceSubtype()));
	}


	ReturnValue_t serialize(uint8_t** buffer, uint32_t* size,
			const uint32_t max_size, bool bigEndian) const {
		ReturnValue_t result = AutoSerializeAdapter::serialize(&apid,buffer,size,max_size,bigEndian);
		if(result!=HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = AutoSerializeAdapter::serialize(&sourceSequenceCount,buffer,size,max_size,bigEndian);
		if(result!=HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = AutoSerializeAdapter::serialize(&serviceType,buffer,size,max_size,bigEndian);
		if(result!=HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = AutoSerializeAdapter::serialize(&serviceSubtype,buffer,size,max_size,bigEndian);
		if(result!=HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		result = AutoSerializeAdapter::serialize(&subCounter,buffer,size,max_size,bigEndian);
		if(result!=HasReturnvaluesIF::RETURN_OK){
			return result;
		}
		SerialBufferAdapter<uint8_t> adapter(rawTimestamp,sizeof(rawTimestamp));
		return adapter.serialize(buffer,size,max_size,bigEndian);
	}

	uint32_t getSerializedSize() const {
		uint32_t size = 0;
		size += AutoSerializeAdapter::getSerializedSize(&apid);
		size += AutoSerializeAdapter::getSerializedSize(&sourceSequenceCount);
		size += AutoSerializeAdapter::getSerializedSize(&serviceType);
		size += AutoSerializeAdapter::getSerializedSize(&serviceSubtype);
		size += AutoSerializeAdapter::getSerializedSize(&subCounter);
		SerialBufferAdapter<uint8_t> adapter(rawTimestamp,sizeof(rawTimestamp));
		size += adapter.getSerializedSize();
		return size;

	};

	ReturnValue_t deSerialize(const uint8_t** buffer, int32_t* size,
	bool bigEndian) {
		ReturnValue_t result = AutoSerializeAdapter::deSerialize(&apid, buffer,
				size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = AutoSerializeAdapter::deSerialize(&sourceSequenceCount, buffer,
				size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = AutoSerializeAdapter::deSerialize(&serviceType, buffer, size,
				bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = AutoSerializeAdapter::deSerialize(&serviceSubtype, buffer,
				size, bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		result = AutoSerializeAdapter::deSerialize(&subCounter, buffer, size,
				bigEndian);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
		SerialBufferAdapter<uint8_t> adapter(rawTimestamp,sizeof(rawTimestamp));
		return adapter.deSerialize(buffer,size,bigEndian);
	}

private:
	uint16_t apid;
	uint16_t sourceSequenceCount;
	uint8_t serviceType;
	uint8_t serviceSubtype;
	uint8_t subCounter;
	uint8_t rawTimestamp[TimeStamperIF::MISSION_TIMESTAMP_SIZE];

};
#endif /* FRAMEWORK_TMSTORAGE_TMSTOREPACKETS_H_ */
