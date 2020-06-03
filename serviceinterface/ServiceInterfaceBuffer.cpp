#include <framework/timemanager/Clock.h>
#include <framework/serviceinterface/ServiceInterfaceBuffer.h>
#include <cstring>

// to be implemented by bsp
extern "C" void printChar(const char*, bool errStream);

#ifndef UT699

ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string setMessage,
        bool errStream, bool addCrToPreamble, uint16_t port):
		isActive(true), logMessage(setMessage),
		addCrToPreamble(addCrToPreamble), errStream(errStream) {
	if(not errStream) {
		// Set pointers if the stream is buffered.
		setp( buf, buf + BUF_SIZE );
	}
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {
	char array[BUF_SIZE];
	uint32_t length = end - begin;
	if (length > sizeof(array)) {
		length = sizeof(array);
	}
	memcpy(array, begin, length);

	for(; begin != end; begin++){
		printChar(begin, false);
	}
}

#endif

int ServiceInterfaceBuffer::overflow(int c) {
	if(errStream and this->isActive) {
		if (c != Traits::eof()) {
			printChar(reinterpret_cast<const char*>(&c), true);
		}
		return 0;
	}
	// Handle output
	putChars(pbase(), pptr());
	if (c != Traits::eof()) {
		char c2 = c;
		// Handle the one character that didn't fit to buffer
		putChars(&c2, &c2 + 1);
	}
	// This tells that buffer is empty again
	setp(buf, buf + BUF_SIZE - 1);
	// I'm not sure about this return value!
	return 0;
}

int ServiceInterfaceBuffer::sync(void) {
	if(not this->isActive or errStream) {
		if(not errStream) {
			setp(buf, buf + BUF_SIZE - 1);
		}
		return 0;
	}

	auto preamble = getPreamble();
	// Write logMessage and time
	this->putChars(preamble.c_str(), preamble.c_str() + preamble.size());
	// Handle output
	this->putChars(pbase(), pptr());
	// This tells that buffer is empty again
	setp(buf, buf + BUF_SIZE - 1);
	return 0;
}


std::string ServiceInterfaceBuffer::getPreamble() {
	Clock::TimeOfDay_t loggerTime;
	Clock::getDateAndTime(&loggerTime);
	std::string preamble;
	if(addCrToPreamble) {
		preamble += "\r";
	}
	preamble += logMessage + ": | " + zero_padded(loggerTime.hour, 2)
							+ ":" + zero_padded(loggerTime.minute, 2) + ":"
							+ zero_padded(loggerTime.second, 2) + "."
							+ zero_padded(loggerTime.usecond/1000, 3) + " | ";
	return preamble;
}



#ifdef UT699
#include <framework/osal/rtems/Interrupt.h>

ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string set_message,
		uint16_t port) {
	this->log_message = set_message;
	this->isActive = true;
	setp( buf, buf + BUF_SIZE );
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {
	char array[BUF_SIZE];
	uint32_t length = end - begin;
	if (length > sizeof(array)) {
		length = sizeof(array);
	}
	memcpy(array, begin, length);

	if (!Interrupt::isInterruptInProgress()) {
		std::cout << array;
	} else {
		//Uncomment the following line if you need ISR debug output.
//		printk(array);
	}
}
#endif //UT699

#ifdef ML505
#include <bsp_flp/network/networkconfig.h>
ServiceInterfaceBuffer::ServiceInterfaceBuffer(std::string set_message,
		uint16_t port) :
		isActive(true), log_message(set_message), udpSocket(0), remoteAddressLength(
				sizeof(remoteAddress)) {
	setp(buf, buf + BUF_SIZE);
	memset((uint8_t*) &remoteAddress, 0, sizeof(remoteAddress));
	remoteAddress.sin_family = AF_INET;
	remoteAddress.sin_port = htons(port);
	remoteAddress.sin_addr.s_addr = htonl(inet_addr("192.168.250.100"));
}

void ServiceInterfaceBuffer::putChars(char const* begin, char const* end) {

	char array[BUF_SIZE];
	uint32_t length = end - begin;
	if (length > sizeof(array)) {
		length = sizeof(array);
	}
	memcpy(array, begin, length);

	if (udpSocket <= 0) {
		initSocket();
	}

	if (udpSocket > 0) {
		sendto(udpSocket, array, length, 0, (sockaddr*) &remoteAddress,
				sizeof(remoteAddress));
	}

}

void ServiceInterfaceBuffer::initSocket() {
	sockaddr_in address;
	memset((uint8_t*) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(0);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket < 0) {
		printf("Error opening socket!\n");
		return;
	}
	timeval timeout = { 0, 20 };
	if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		printf("Error setting SO_RCVTIMEO socket options!\n");
		return;
	}
	if (setsockopt(udpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout,
			sizeof(timeout)) < 0) {
		printf("Error setting SO_SNDTIMEO socket options!\n");
		return;
	}
	if (bind(udpSocket, (sockaddr *) &address, sizeof(address)) < 0) {
		printf("Error binding socket!\n");
	}
}

#endif //ML505
