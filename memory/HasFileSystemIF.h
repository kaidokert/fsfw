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

    static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING = MAKE_RETURN_CODE(0x05);

	virtual ~HasFileSystemIF() {}
	/**
	 * Function to get the MessageQueueId_t of the implementing object
	 * @return MessageQueueId_t of the object
	 */
	virtual MessageQueueId_t getCommandQueue() const = 0;

	/**
	 * Generic function to append to file.
	 * @param dirname Directory of the file
	 * @param filename The filename of the file
	 * @param data The data to write to the file
	 * @param size The size of the data to write
	 * @param packetNumber Current packet number. Can be used to verify that
	 * there are no missing packets.
	 * @param args Any other arguments which an implementation might require.
	 * @param bytesWritten Actual bytes written to file
	 * For large files the write procedure must be split in multiple calls
	 * to writeToFile
	 */
	virtual ReturnValue_t appendToFile(const char* repositoryPath,
	        const char* filename, const uint8_t* data, size_t size,
	        uint16_t packetNumber, void* args = nullptr) = 0;

	/**
	 * Generic function to create a new file.
	 * @param repositoryPath
	 * @param filename
	 * @param data
	 * @param size
	 * @param args Any other arguments which an implementation might require.
	 * @return
	 */
	virtual ReturnValue_t createFile(const char* repositoryPath,
	        const char* filename, const uint8_t* data = nullptr,
	        size_t size = 0, void* args = nullptr) = 0;

	/**
	 * Generic function to delete a file.
	 * @param repositoryPath
	 * @param filename
	 * @param args
	 * @return
	 */
	virtual ReturnValue_t deleteFile(const char* repositoryPath,
	        const char* filename, void* args = nullptr) = 0;
};


#endif /* FSFW_MEMORY_HASFILESYSTEMIF_H_ */
