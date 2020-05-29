#include <framework/serviceinterface/ServiceInterfaceStream.h>

void ServiceInterfaceStream::setActive( bool myActive) {
	this->buf.isActive = myActive;
}

ServiceInterfaceStream::ServiceInterfaceStream(std::string set_message,
		bool addCrToPreamble, uint16_t port) :
		std::basic_ostream<char, std::char_traits<char>>(&buf),
		buf(set_message, port, addCrToPreamble) {
}
