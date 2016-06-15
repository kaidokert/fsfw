/**
 * @file	LinkedElementDecorator.h
 * @brief	This file defines the LinkedElementDecorator class.
 * @date	22.07.2014
 * @author	baetz
 */
#ifndef LINKEDELEMENTDECORATOR_H_
#define LINKEDELEMENTDECORATOR_H_

#include <framework/container/SinglyLinkedList.h>
#include <utility>

//TODO: This generates multiple inheritance from non-IF parents.
template<typename T, typename IF_T>
class LinkedElementDecorator : public LinkedElement<IF_T>, public T {
public:
	template<typename... Args>
	LinkedElementDecorator(Args... args) : LinkedElement<IF_T>(this), T(std::forward<Args>(args)...) {
	}

	virtual ~LinkedElementDecorator() {
	}
};



#endif /* LINKEDELEMENTDECORATOR_H_ */
