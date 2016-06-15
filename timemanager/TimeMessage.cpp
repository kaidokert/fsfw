/**
 * @file	TimeMessage.cpp
 * @brief	This file defines the TimeMessage class.
 * @date	26.02.2013
 * @author	baetz
 */

#include <framework/timemanager/TimeMessage.h>

TimeMessage::TimeMessage() {
	this->messageSize += sizeof(timeval);
}

TimeMessage::TimeMessage(timeval setTime) {
	memcpy (this->getData(), &setTime, sizeof(timeval));
	this->messageSize += sizeof(timeval);
}

TimeMessage::~TimeMessage() {
}

timeval TimeMessage::getTime() {
	timeval temp;
	memcpy( &temp, this->getData(), sizeof(timeval));
	return temp;
}

size_t TimeMessage::getMinimumMessageSize() {
	return this->MAX_SIZE;
}
