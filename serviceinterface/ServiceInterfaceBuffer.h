#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACEBUFFER_H_

#include <iostream>
#include <iosfwd>
#include <sstream>
#include <cstdio>

#ifndef UT699
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
