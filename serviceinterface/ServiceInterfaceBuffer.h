#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifndef UT699

/**
 * @brief 	This is the underlying stream buffer which implements the
 * 			streambuf class and overloads the overflow() and sync() methods
 * @details
 * This class is used to modify the output of the stream, for example by adding.
 * It also calls the char printing function which is implemented in the
 * board supply package (BSP).
 */
class ServiceInterfaceBuffer:
        public std::streambuf {
	friend class ServiceInterfaceStream;
public:
	ServiceInterfaceBuffer(std::string setMessage, bool addCrToPreamble,
			bool buffered, bool errStream, uint16_t port);

protected:
	bool isActive;
	//! This is called when buffer becomes full. If
	//! buffer is not used, then this is called every
	//! time when characters are put to stream.
	int overflow(int c = Traits::eof()) override;

	//! This function is called when stream is flushed,
	//! for example when std::endl is put to stream.
	int sync(void) override;

private:
	//! For additional message information
	std::string logMessage;
	std::string preamble;
	// For EOF detection
	typedef std::char_traits<char> Traits;
	//! This is useful for some terminal programs which do not have
	//! implicit carriage return with newline characters.
	bool addCrToPreamble;
	bool buffered;
	//! This specifies to print to stderr and work in unbuffered mode.
	bool errStream;

	// Work in buffer mode. It is also possible to work without buffer.
	static size_t const BUF_SIZE = 128;
	char buf[BUF_SIZE];

	//! In this function, the characters are parsed.
	void putChars(char const* begin, char const* end);

	std::string getPreamble(size_t * preambleSize = nullptr);

	/**
	 * This helper function returns the zero padded string version of a number.
	 * The type is deduced automatically.
	 * TODO: This uses dynamic memory allocation, so we should provide
	 * a custom streambuf class to use it (which takes maxSize as argument)
	 * Then we would propably
	 * @tparam T
	 * @param num
	 * @param width
	 * @return
	 */
	template<typename T>
	ReturnValue_t zeroPadded(std::string stringToFill, const T& num,
			uint8_t width) {
		auto numString = std::to_string(num);
		uint8_t i = 0;
		for(i = 0; i < width; i++) {
			stringToFill[i] = '0';
		}
		numString.copy(stringToFill.data() + i, numString.size());
		return HasReturnvaluesIF::RETURN_OK;
	}
};

#endif


#ifdef UT699
class ServiceInterfaceBuffer: public std::basic_streambuf<char,
		std::char_traits<char> > {
	friend class ServiceInterfaceStream;
public:
	ServiceInterfaceBuffer(std::string set_message, uint16_t port);
protected:
	bool isActive;
	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	virtual int overflow(int c = Traits::eof());

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	virtual int sync(void);

private:
	// For additional message information
	std::string log_message;
	// For EOF detection
	typedef std::char_traits<char> Traits;

	// Work in buffer mode. It is also possible to work without buffer.
	static size_t const BUF_SIZE = 128;
	char buf[BUF_SIZE];

	// In this function, the characters are parsed.
	void putChars(char const* begin, char const* end);
};
#endif //UT699

#ifdef ML505
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/udp.h>

class ServiceInterfaceBuffer: public std::basic_streambuf<char,
		std::char_traits<char> > {
	friend class ServiceInterfaceStream;
public:
	ServiceInterfaceBuffer(std::string set_message, uint16_t port);
protected:
	bool isActive;
	// This is called when buffer becomes full. If
	// buffer is not used, then this is called every
	// time when characters are put to stream.
	virtual int overflow(int c = Traits::eof());

	// This function is called when stream is flushed,
	// for example when std::endl is put to stream.
	virtual int sync(void);

private:
	// For additional message information
	std::string log_message;
	// For EOF detection
	typedef std::char_traits<char> Traits;

	// Work in buffer mode. It is also possible to work without buffer.
	static size_t const BUF_SIZE = 128;
	char buf[BUF_SIZE];

	// In this function, the characters are parsed.
	void putChars(char const* begin, char const* end);

	int udpSocket;
	sockaddr_in remoteAddress;
	socklen_t remoteAddressLength;
	void initSocket();
};
#endif //ML505


#endif /* FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_ */
