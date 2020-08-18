/*
 * HasFileSystemIF.h
 *
 *  Created on: 19.01.2020
 *      Author: Jakob Meier
 */

#ifndef FRAMEWORK_MEMORY_HASFILESYSTEMIF_H_
#define FRAMEWORK_MEMORY_HASFILESYSTEMIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"

class HasFileSystemIF {
public:

	virtual ~HasFileSystemIF() {}
	/**
	 * Function to get the MessageQueueId_t of the implementing object
	 * @return MessageQueueId_t of the object
	 */
	virtual MessageQueueId_t getCommandQueue() const = 0;
	/**
	 * Function to write to a file
	 * @param dirname Directory of the file
	 * @param filename The filename of the file
	 * @param data The data to write to the file
	 * @param size The size of the data to write
	 * @param packetNumber Counts the number of packets. For large files the write procedure must be split in multiple calls to writeToFile
	 */
	virtual ReturnValue_t writeToFile(const char* dirname, char* filename, const uint8_t* data, uint32_t size, uint16_t packetNumber) = 0;
	virtual ReturnValue_t createFile(const char* dirname, const char* filename, const uint8_t* data, uint32_t size) = 0;
	virtual ReturnValue_t deleteFile(const char* dirname, const char* filename) = 0;
};


#endif /* FRAMEWORK_MEMORY_HASFILESYSTEMIF_H_ */
