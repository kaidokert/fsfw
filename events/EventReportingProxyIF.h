/*
 * EventReportingProxyIF.h
 *
 *  Created on: 19.08.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_EVENTS_EVENTREPORTINGPROXYIF_H_
#define FRAMEWORK_EVENTS_EVENTREPORTINGPROXYIF_H_

#include <framework/events/Event.h>


class EventReportingProxyIF {
public:
	virtual ~EventReportingProxyIF() {
	}

	virtual void forwardEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0) const = 0;

};




#endif /* FRAMEWORK_EVENTS_EVENTREPORTINGPROXYIF_H_ */
