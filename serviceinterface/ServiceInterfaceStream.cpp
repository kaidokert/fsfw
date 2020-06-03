#include <framework/serviceinterface/ServiceInterfaceStream.h>

ServiceInterfaceStream::ServiceInterfaceStream(std::string setMessage,
		bool errStream, bool addCrToPreamble, uint16_t port) :
		std::ostream(&buf),
		buf(setMessage, errStream, addCrToPreamble, port) {
}

void ServiceInterfaceStream::setActive( bool myActive) {
	this->buf.isActive = myActive;
}

std::string ServiceInterfaceStream::getPreamble() {
	return buf.getPreamble();
}
