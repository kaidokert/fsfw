#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_

#include <framework/serviceinterface/ServiceInterfaceBuffer.h>
#include <iostream>
#include <cstdio>

/* Unfortunately, there must be a forward declaration of the log front end
 * (MUST be defined in main), to let the system know where to write to.
 * The ServiceInterfaceStream instances, which are declared below and
 * can instantaited somewhere else can be passed to these front ends by passing
 * their underlying buffers with .rdbuf() */
namespace sif {
extern std::ostream debug;
extern std::ostream info;
extern std::ostream warning;
extern std::ostream error;
}


class ServiceInterfaceStream :
        public std::ostream {
protected:
    ServiceInterfaceBuffer buf;
public:
    /**
     * This constructor is used by specifying the preamble message.
     * Optionally, the output can be directed to stderr and a CR character
     * can be prepended to the preamble.
     * @param set_message
     * @param errStream
     * @param addCrToPreamble
     * @param port
     */
    ServiceInterfaceStream(std::string setMessage,
    		bool errStream = false, bool addCrToPreamble = false,
			uint16_t port = 1234);

    //! An inactive stream will not print anything.
	void setActive( bool );

	/**
	 * This can be used to retrieve the preamble in case it should be printed in
	 * the unbuffered mode.
	 * @return Preamle consisting of log message and timestamp.
	 */
	std::string getPreamble();
};

// Forward declaration of interface streams. These are needed so the public
// functions can be used by including this header
namespace sif {
extern ServiceInterfaceStream debugStream;
extern ServiceInterfaceStream infoStream;
extern ServiceInterfaceStream warningStream;
extern ServiceInterfaceStream errorStream;
}

#endif /* FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_ */
