/**
 * @file	ExecutableObjectIF.h
 *
 * @brief	This file contains the definition for the ExecutableObjectIF interface.
 *
 * @author	Bastian Baetz
 *
 * @date	12.03.2012
 */

#ifndef EXECUTABLEOBJECTIF_H_
#define EXECUTABLEOBJECTIF_H_


#include <framework/osal/OSAL.h>

/**
 * @brief	The interface provides a method to execute objects within a task.
 * @details	The performOperation method, that is required by the interface is
 * 			executed cyclically within the ObjectTask's context.
 */
class ExecutableObjectIF {
public:
	/**
	 * @brief	This is the empty virtual destructor as required for C++ interfaces.
	 */
	virtual ~ExecutableObjectIF() { }
	/**
	 * @brief	The performOperation method is executed in the ObjectTask context.
	 * @details	There are no restrictions for calls within this method, so any
	 * 			other member of the class can be used.
	 * @return	Currently, the return value is ignored.
	 */
	virtual ReturnValue_t performOperation() = 0;
};

#endif /* EXECUTABLEOBJECTIF_H_ */
