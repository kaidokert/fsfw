/**
 * \file Service8Packets.h
 *
 * \brief Structure of a Direct Command.
 * Normal reply (subservice 130) consists of
 *   1. Target object ID
 *   2. Action ID (taget device has specified functions with action IDs)
 *   3. Return Code
 *   4. Optional step number for step replies
 *
 * Data reply (subservice 132) consists of
 *   1. Target Object ID
 *   2. Action ID
 *   3. Data
 *
 *  \date 01.07.2019
 *  \author R. Mueller
 */

#ifndef FRAMEWORK_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_
#define FRAMEWORK_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_

#include <framework/action/ActionMessage.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/serialize/SerialBufferAdapter.h>
#include <framework/serialize/SerializeElement.h>
#include <framework/serialize/SerialLinkedListAdapter.h>
#include <framework/serialize/SerialFixedArrayListAdapter.h>


/**
 * \brief Subservice 128
 * \ingroup spacepackets
 */
class DirectCommand: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 128
public:
	//typedef uint16_t typeOfMaxData;
	//static const typeOfMaxData MAX_DATA = 256;
	DirectCommand(const uint8_t* dataBuffer_, uint32_t size_) {
		size_t size = sizeof(objectId);
		SerializeAdapter::deSerialize(&objectId,&dataBuffer_,&size,
		        SerializeIF::Endianness::BIG);
		size = sizeof(actionId);
		SerializeAdapter::deSerialize(&actionId,&dataBuffer_,&size,
		        SerializeIF::Endianness::BIG);
		parameterBuffer = dataBuffer_;
		parametersSize = size_ - sizeof(objectId) - sizeof(actionId);
	}
	ActionId_t getActionId() const {
		return actionId;
	}

	object_id_t getObjectId() const {
		return objectId;
	}

	const uint8_t*  getParameters() {
		return parameterBuffer;
	}

	uint32_t getParametersSize() const {
		return parametersSize;
	}

private:
	DirectCommand(const DirectCommand &command);
	object_id_t objectId;
	ActionId_t actionId;
	uint32_t parametersSize; //!< [EXPORT] : [IGNORE]
	const uint8_t * parameterBuffer; //!< [EXPORT] : [MAXSIZE] 65535 Bytes

};


/**
 * \brief Subservice 130
 * \ingroup spacepackets
 */
class DataReply: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 130
public:
	typedef uint16_t typeOfMaxDataSize;
	static const uint16_t MAX_DATA_LENGTH = sizeof(typeOfMaxDataSize);
	DataReply(object_id_t objectId_, ActionId_t actionId_,
		const uint8_t * replyDataBuffer_ = NULL, uint16_t replyDataSize_ = 0):
		objectId(objectId_), actionId(actionId_), replyData(replyDataBuffer_,replyDataSize_){
		setLinks();
	}

private:
	DataReply(const DataReply &reply);
	void setLinks() {
		setStart(&objectId);
		objectId.setNext(&actionId);
		actionId.setNext(&replyData);
	}
	SerializeElement<object_id_t> objectId;
	SerializeElement<ActionId_t> actionId;
	SerializeElement<SerialBufferAdapter<uint16_t>> replyData;
};


/**
 * \brief Subservice 132
 * \ingroup spacepackets
 */
class DirectReply: public SerialLinkedListAdapter<SerializeIF> { //!< [EXPORT] : [SUBSERVICE] 132
public:
	typedef uint16_t typeOfMaxDataSize;
	static const uint16_t MAX_DATA_LENGTH = sizeof(typeOfMaxDataSize);

	DirectReply(object_id_t objectId_, ActionId_t actionId_, ReturnValue_t returnCode_,
			bool isStep_ = false, uint8_t step_ = 0):
			isStep(isStep_), objectId(objectId_), actionId(actionId_),
			returnCode(returnCode_),step(step_)  {
		setLinks();
	}
private:

	void setLinks() {
		setStart(&objectId);
		objectId.setNext(&actionId);
		actionId.setNext(&returnCode);
		if(isStep) {
			returnCode.setNext(&step);
		}
	}
	bool isDataReply; //!< [EXPORT] : [IGNORE]
	bool isStep; //!< [EXPORT] : [IGNORE]
	SerializeElement<object_id_t> objectId; //!< [EXPORT] : [IGNORE]
	SerializeElement<ActionId_t> actionId; //!< [EXPORT] : [IGNORE]
	SerializeElement<ReturnValue_t> returnCode; //!< [EXPORT] : [IGNORE]
	SerializeElement<uint8_t> step; //!< [EXPORT] : [OPTIONAL] [IGNORE]

};

#endif /* FRAMEWORK_PUS_SERVICEPACKETS_SERVICE8PACKETS_H_ */
