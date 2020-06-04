#include <framework/serviceinterface/ServiceInterfaceStream.h>

ServiceInterfaceStream::ServiceInterfaceStream(std::string setMessage,
		bool addCrToPreamble, bool buffered, bool errStream, uint16_t port) :
		std::ostream(&buf),
		buf(setMessage, addCrToPreamble, buffered, errStream,  port) {
}

void ServiceInterfaceStream::setActive( bool myActive) {
	this->buf.isActive = myActive;
}

std::string ServiceInterfaceStream::getPreamble() {
	return buf.getPreamble();
}

void ServiceInterfaceStream::print(std::string error,
		bool withPreamble, bool withNewline, bool flush) {
	if(not buffered and withPreamble) {
		*this << getPreamble() << error;
	}
	else {
		*this << error;
	}

	if(withNewline) {
		*this << "\n";
	}
	// if mode is non-buffered, no need to flush.
	if(flush and buffered) {
		this->flush();
	}
}
