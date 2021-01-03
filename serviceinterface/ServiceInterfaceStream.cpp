#include "ServiceInterfaceStream.h"

ServiceInterfaceStream::ServiceInterfaceStream(std::string setMessage,
		bool addCrToPreamble, bool buffered, bool errStream, uint16_t port) :
		std::ostream(&streambuf),
		streambuf(setMessage, addCrToPreamble, buffered, errStream, port) {}

void ServiceInterfaceStream::setActive( bool myActive) {
	this->streambuf.isActive = myActive;
}

std::string* ServiceInterfaceStream::getPreamble() {
	return streambuf.getPreamble();
}

