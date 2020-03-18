#ifndef COOKIE_H_
#define COOKIE_H_

/**
 * @brief This datatype is used to identify different connection over a single interface
 *        (like RMAP or I2C)
 * @details
 * To use this class, implement a communication specific child cookie. This cookie
 * can be used in the device communication interface by performing
 * a C++ dynamic cast. The cookie can be used to store all kinds of information
 * about the communication between read and send calls.
 */
class Cookie{
public:
	virtual ~Cookie(){}
};


#endif /* COOKIE_H_ */
