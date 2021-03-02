#ifndef FSFW_MEMORY_HASFILESYSTEMIF_H_
#define FSFW_MEMORY_HASFILESYSTEMIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include "../returnvalues/FwClassIds.h"
#include "../ipc/messageQueueDefinitions.h"

#include <cstddef>

/**
 * @brief	Generic interface for objects which expose a file system to enable
 * 			message based file handling.
 * @author  J. Meier, R. Mueller
 */
class HasFileSystemIF {
public:
    static constexpr uint8_t INTERFACE_ID = CLASS_ID::FILE_SYSTEM;

    //! [EXPORT] : P1: Can be file system specific error code
    static constexpr ReturnValue_t GENERIC_FILE_ERROR = MAKE_RETURN_CODE(0);

    static constexpr ReturnValue_t FILE_DOES_NOT_EXIST = MAKE_RETURN_CODE(1);
    static constexpr ReturnValue_t FILE_ALREADY_EXISTS = MAKE_RETURN_CODE(2);
    static constexpr ReturnValue_t FILE_LOCKED = MAKE_RETURN_CODE(3);

    static constexpr ReturnValue_t DIRECTORY_DOES_NOT_EXIST = MAKE_RETURN_CODE(4);
    static constexpr ReturnValue_t DIRECTORY_ALREADY_EXISTS = MAKE_RETURN_CODE(5);
    static constexpr ReturnValue_t DIRECTORY_NOT_EMPTY = MAKE_RETURN_CODE(6);

    //! [EXPORT] : P1: Sequence number missing
    static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_WRITE = MAKE_RETURN_CODE(7);
    //! [EXPORT] : P1: Sequence number missing
    static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_READ = MAKE_RETURN_CODE(8);

    //! [EXPORT] : File system is currently busy
    static constexpr ReturnValue_t IS_BUSY = MAKE_RETURN_CODE(9);
    //! [EXPORT] : Invalid parameters like file name or repository path
    static constexpr ReturnValue_t INVALID_PARAMETERS = MAKE_RETURN_CODE(10);

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
