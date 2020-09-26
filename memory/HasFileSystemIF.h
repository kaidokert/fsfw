#ifndef FSFW_MEMORY_HASFILESYSTEMIF_H_
#define FSFW_MEMORY_HASFILESYSTEMIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../returnvalues/FwClassIds.h"
#include "../ipc/messageQueueDefinitions.h"

#include <cstddef>

/**
 * @author  Jakob Meier
 */
class HasFileSystemIF {
public:
    static constexpr uint8_t INTERFACE_ID = CLASS_ID::FILE_SYSTEM;

    static constexpr ReturnValue_t FILE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x00);
    static constexpr ReturnValue_t FILE_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);

    static constexpr ReturnValue_t DIRECTORY_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x02);
    static constexpr ReturnValue_t DIRECTORY_ALREADY_EXISTS = MAKE_RETURN_CODE(0x03);
    static constexpr ReturnValue_t DIRECTORY_NOT_EMPTY = MAKE_RETURN_CODE(0x04);

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
	 * @param packetNumber Counts the number of packets.
	 * For large files the write procedure must be split in multiple calls
	 * to writeToFile
	 */
	virtual ReturnValue_t writeToFile(const char* dirname, const char* filename,
	        const uint8_t* data, size_t size, uint16_t packetNumber) = 0;
	virtual ReturnValue_t createFile(const char* dirname, const char* filename,
	        const uint8_t* data = nullptr, size_t size = 0,
	        size_t* bytesWritten = nullptr) = 0;
	virtual ReturnValue_t deleteFile(const char* dirname,
	        const char* filename) = 0;
};


#endif /* FSFW_MEMORY_HASFILESYSTEMIF_H_ */
