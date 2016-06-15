/*
 * PrivatePoolIF.h
 *
 *  Created on: 20.03.2014
 *      Author: baetz
 */

#ifndef PRIVATEPOOLIF_H_
#define PRIVATEPOOLIF_H_
#include <framework/memory/HasMemoryIF.h>
#include <framework/serialize/SerializeIF.h>

class PrivatePoolIF : public HasMemoryIF, public SerializeIF {
public:
	virtual ~PrivatePoolIF() {

	}
};



#endif /* PRIVATEPOOLIF_H_ */
