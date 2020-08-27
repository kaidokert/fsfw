#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_

#include "ServiceInterfaceBuffer.h"
#include <iostream>
#include <cstdio>

/**
 * Generic service interface stream which can be used like std::cout or
 * std::cerr but has additional capability. Add preamble and timestamp
 * to output. Can be run in buffered or unbuffered mode.
 */
class ServiceInterfaceStream : public std::ostream {
public:
	/**
	 * This constructor is used by specifying the preamble message.
	 * Optionally, the output can be directed to stderr and a CR character
	 * can be prepended to the preamble.
	 * @param setMessage message of preamble.
	 * @param addCrToPreamble Useful for applications like Puttty.
	 * @param buffered specify whether to use buffered mode.
	 * @param errStream specify which output stream to use (stderr or stdout).
	 */
	ServiceInterfaceStream(std::string setMessage,
			bool addCrToPreamble = false, bool buffered = true,
			bool errStream = false, uint16_t port = 1234);

	//! An inactive stream will not print anything.
	void setActive( bool );

	/**
	 * This can be used to retrieve the preamble in case it should be printed in
	 * the unbuffered mode.
	 * @return Preamle consisting of log message and timestamp.
	 */
	std::string* getPreamble();

	/**
	 * This prints an error with a preamble. Useful if using the unbuffered
	 * mode. Flushes in default mode (prints immediately).
	 */
	void print(std::string error, bool withPreamble = true,
			bool withNewline = true, bool flush = true);

protected:
	ServiceInterfaceBuffer streambuf;
};

// Forward declaration of interface streams. These should be instantiated in
// main. They can then be used like std::cout or std::cerr.
namespace sif {
extern ServiceInterfaceStream debug;
extern ServiceInterfaceStream info;
extern ServiceInterfaceStream warning;
extern ServiceInterfaceStream error;
}

#endif /* FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_ */
