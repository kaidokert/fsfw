#ifndef FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_
#define FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_

#include <framework/serviceinterface/ServiceInterfaceBuffer.h>
#include <iostream>
#include <iosfwd>
#include <sstream>
#include <cstdio>

// Unfortunately, there must be a forward declaration of log_fe
// (MUST be defined in main), to let the system know where to write to.
extern std::ostream debug;
extern std::ostream info;
extern std::ostream warning;
extern std::ostream error;

class ServiceInterfaceStream :
        public std::basic_ostream<char, std::char_traits<char>> {
protected:
    ServiceInterfaceBuffer buf;
public:
    ServiceInterfaceStream( std::string set_message,
    		bool addCrToPreamble = false, uint16_t port = 1234);
	void setActive( bool );
};


#endif /* FRAMEWORK_SERVICEINTERFACE_SERVICEINTERFACESTREAM_H_ */
