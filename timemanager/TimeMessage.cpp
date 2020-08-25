/**
 * @file	TimeMessage.cpp
 * @brief	This file defines the TimeMessage class.
 * @date	26.02.2013
 * @author	baetz
 */

#include "TimeMessage.h"

TimeMessage::TimeMessage() {
	this->messageSize += sizeof(timeval) + sizeof(uint32_t);
}

TimeMessage::TimeMessage(timeval setTime, uint32_t CounterValue) {
	memcpy (this->getData(), &setTime, sizeof(timeval));
	this->messageSize += sizeof(timeval) + sizeof(uint32_t);
	memcpy (this->getData() + sizeof(timeval), &CounterValue, sizeof(uint32_t));
}

TimeMessage::~TimeMessage() {
}

timeval TimeMessage::getTime() {
	timeval temp;
	memcpy( &temp, this->getData(), sizeof(timeval));
	return temp;
}

uint32_t TimeMessage::getCounterValue() {
	uint32_t temp;
	memcpy ( &temp, this->getData() + sizeof(timeval), sizeof(uint32_t));
	return temp;
}

size_t TimeMessage::getMinimumMessageSize() {
	return this->MAX_SIZE;
}
