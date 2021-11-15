#ifndef FSFW_MEMORY_HASFILESYSTEMIF_H_
#define FSFW_MEMORY_HASFILESYSTEMIF_H_

#include "FileSystemArgsIF.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/ipc/messageQueueDefinitions.h"

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
    //! [EXPORT] : File system is currently busy
    static constexpr ReturnValue_t IS_BUSY = MAKE_RETURN_CODE(1);
    //! [EXPORT] : Invalid parameters like file name or repository path
    static constexpr ReturnValue_t INVALID_PARAMETERS = MAKE_RETURN_CODE(2);

    static constexpr ReturnValue_t FILE_DOES_NOT_EXIST = MAKE_RETURN_CODE(5);
    static constexpr ReturnValue_t FILE_ALREADY_EXISTS = MAKE_RETURN_CODE(6);
    static constexpr ReturnValue_t FILE_LOCKED = MAKE_RETURN_CODE(7);

    static constexpr ReturnValue_t DIRECTORY_DOES_NOT_EXIST = MAKE_RETURN_CODE(10);
    static constexpr ReturnValue_t DIRECTORY_ALREADY_EXISTS = MAKE_RETURN_CODE(11);
    static constexpr ReturnValue_t DIRECTORY_NOT_EMPTY = MAKE_RETURN_CODE(12);

    //! [EXPORT] : P1: Sequence number missing
    static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_WRITE = MAKE_RETURN_CODE(15);
    //! [EXPORT] : P1: Sequence number missing
    static constexpr ReturnValue_t SEQUENCE_PACKET_MISSING_READ = MAKE_RETURN_CODE(16);

	virtual ~HasFileSystemIF() {}

	/**
	 * Function to get the MessageQueueId_t of the implementing object
	 * @return MessageQueueId_t of the object
	 */
	virtual MessageQueueId_t getCommandQueue() const = 0;

	/**
	 * @brief   Generic function to append to file.
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
	        uint16_t packetNumber, FileSystemArgsIF* args = nullptr) = 0;

	/**
	 * @brief   Generic function to create a new file.
	 * @param repositoryPath
	 * @param filename
	 * @param data
	 * @param size
	 * @param args Any other arguments which an implementation might require
	 * @return
	 */
	virtual ReturnValue_t createFile(const char* repositoryPath,
	        const char* filename, const uint8_t* data = nullptr,
	        size_t size = 0, FileSystemArgsIF* args = nullptr) = 0;

	/**
	 * @brief   Generic function to delete a file.
	 * @param repositoryPath
	 * @param filename
	 * @param args Any other arguments which an implementation might require
	 * @return
	 */
	virtual ReturnValue_t removeFile(const char* repositoryPath,
	        const char* filename, FileSystemArgsIF* args = nullptr) = 0;

	/**
	 * @brief   Generic function to create a directory
	 * @param repositoryPath
	 * @param   Equivalent to the -p flag in Unix systems. If some required parent directories
	 *          do not exist, create them as well
	 * @param args Any other arguments which an implementation might require
	 * @return
	 */
	virtual ReturnValue_t createDirectory(const char* repositoryPath, const char* dirname,
	        bool createParentDirs, FileSystemArgsIF* args = nullptr) = 0;

	/**
	 * @brief   Generic function to remove a directory
	 * @param repositoryPath
     * @param args Any other arguments which an implementation might require
	 */
	virtual ReturnValue_t removeDirectory(const char* repositoryPath, const char* dirname,
	        bool deleteRecurively = false, FileSystemArgsIF* args = nullptr) = 0;

	virtual ReturnValue_t renameFile(const char* repositoryPath, const char* oldFilename,
	        const char* newFilename, FileSystemArgsIF* args = nullptr) = 0;
};


#endif /* FSFW_MEMORY_HASFILESYSTEMIF_H_ */
