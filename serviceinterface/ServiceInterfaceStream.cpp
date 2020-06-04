#include <framework/serviceinterface/ServiceInterfaceStream.h>

ServiceInterfaceStream::ServiceInterfaceStream(std::string setMessage,
		bool errStream, bool addCrToPreamble, uint16_t port) :
		std::ostream(&streambuf),
		streambuf(setMessage, errStream, addCrToPreamble, port) {
}

void ServiceInterfaceStream::setActive( bool myActive) {
	this->streambuf.isActive = myActive;
}

std::string ServiceInterfaceStream::getPreamble() {
	return streambuf.getPreamble();
}
