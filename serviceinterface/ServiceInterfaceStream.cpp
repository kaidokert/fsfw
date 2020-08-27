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

void ServiceInterfaceStream::print(std::string error,
		bool withPreamble, bool withNewline, bool flush) {
	if(not streambuf.isBuffered() and withPreamble) {
		*this << getPreamble() << error;
	}
	else {
		*this << error;
	}

	if(withNewline) {
		*this << "\n";
	}
	// if mode is non-buffered, no need to flush.
	if(flush and streambuf.isBuffered()) {
		this->flush();
	}
}
