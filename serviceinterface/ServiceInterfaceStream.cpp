/*
 * ServiceInterfaceStream.cpp
 *
 *  Created on: 06.08.2015
 *      Author: baetz
 */


#include <framework/serviceinterface/ServiceInterfaceStream.h>

void ServiceInterfaceStream::setActive( bool myActive) {
	this->buf.isActive = myActive;
}

ServiceInterfaceStream::ServiceInterfaceStream(std::string set_message,
		uint16_t port) :
		std::basic_ostream<char, std::char_traits<char> >(&buf), buf(
				set_message, port) {
}
