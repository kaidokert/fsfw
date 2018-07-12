/**
 * @file	ReceivesTimeInfoIF.h
 * @brief	This file defines the ReceivesTimeInfoIF class.
 * @date	26.02.2013
 * @author	baetz
 */

#ifndef RECEIVESTIMEINFOIF_H_
#define RECEIVESTIMEINFOIF_H_

/**
 * This is a Interface for classes that receive timing information
 * with the help of a dedicated message queue.
 */
class ReceivesTimeInfoIF {
public:
	/**
	 * Returns the id of the queue which receives the timing information.
	 * @return Queue id of the timing queue.
	 */
	virtual MessageQueueId_t getTimeReceptionQueue() const = 0;
	/**
	 * Empty virtual destructor.
	 */
	virtual ~ReceivesTimeInfoIF() {
	}

};


#endif /* RECEIVESTIMEINFOIF_H_ */
