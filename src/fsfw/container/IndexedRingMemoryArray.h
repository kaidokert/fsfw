#ifndef FRAMEWORK_CONTAINER_INDEXEDRINGMEMORY_H_
#define FRAMEWORK_CONTAINER_INDEXEDRINGMEMORY_H_

#include <cmath>

#include "../globalfunctions/CRC.h"
#include "../returnvalues/returnvalue.h"
#include "../serialize/SerialArrayListAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "ArrayList.h"

template <typename T>
class Index : public SerializeIF {
  /**
   * Index is the Type used for the list of indices. The template parameter is the type which
   * describes the index, it needs to be a child of SerializeIF to be able to make it persistent
   */
  static_assert(std::is_base_of<SerializeIF, T>::value,
                "Wrong Type for Index, Type must implement SerializeIF");

 public:
  Index() : blockStartAddress(0), size(0), storedPackets(0) {}

  Index(uint32_t startAddress) : blockStartAddress(startAddress), size(0), storedPackets(0) {}

  void setBlockStartAddress(uint32_t newAddress) { this->blockStartAddress = newAddress; }

  uint32_t getBlockStartAddress() const { return blockStartAddress; }

  const T* getIndexType() const { return &indexType; }

  T* modifyIndexType() { return &indexType; }
  /**
   * Updates the index Type. Uses = operator
   * @param indexType Type to copy from
   */
  void setIndexType(T* indexType) { this->indexType = *indexType; }

  uint32_t getSize() const { return size; }

  void setSize(uint32_t size) { this->size = size; }

  void addSize(uint32_t size) { this->size += size; }

  void setStoredPackets(uint32_t newStoredPackets) { this->storedPackets = newStoredPackets; }

  void addStoredPackets(uint32_t packets) { this->storedPackets += packets; }

  uint32_t getStoredPackets() const { return this->storedPackets; }

  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const {
    ReturnValue_t result =
        SerializeAdapter::serialize(&blockStartAddress, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = indexType.serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&this->size, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result =
        SerializeAdapter::serialize(&this->storedPackets, buffer, size, maxSize, streamEndianness);
    return result;
  }

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
    ReturnValue_t result =
        SerializeAdapter::deSerialize(&blockStartAddress, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = indexType.deSerialize(buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&this->size, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::deSerialize(&this->storedPackets, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    return result;
  }

  size_t getSerializedSize() const {
    uint32_t size = SerializeAdapter::getSerializedSize(&blockStartAddress);
    size += indexType.getSerializedSize();
    size += SerializeAdapter::getSerializedSize(&this->size);
    size += SerializeAdapter::getSerializedSize(&this->storedPackets);
    return size;
  }

  bool operator==(const Index<T>& other) {
    return ((blockStartAddress == other.getBlockStartAddress()) && (size == other.getSize())) &&
           (indexType == *(other.getIndexType()));
  }

 private:
  uint32_t blockStartAddress;
  uint32_t size;
  uint32_t storedPackets;
  T indexType;
};

template <typename T>
class IndexedRingMemoryArray : public SerializeIF, public ArrayList<Index<T>, uint32_t> {
  /**
   * Indexed Ring Memory Array is a class for a ring memory with indices. It assumes that the newest
   * data comes in last It uses the currentWriteBlock as pointer to the current writing position The
   * currentReadBlock must be set manually
   */
 public:
  IndexedRingMemoryArray(uint32_t startAddress, uint32_t size, uint32_t bytesPerBlock,
                         SerializeIF* additionalInfo, bool overwriteOld)
      : ArrayList<Index<T>, uint32_t>(NULL, (uint32_t)10, (uint32_t)0),
        totalSize(size),
        indexAddress(startAddress),
        currentReadSize(0),
        currentReadBlockSizeCached(0),
        lastBlockToReadSize(0),
        additionalInfo(additionalInfo),
        overwriteOld(overwriteOld) {
    // Calculate the maximum number of indices needed for this blocksize
    uint32_t maxNrOfIndices = floor(static_cast<double>(size) / static_cast<double>(bytesPerBlock));

    // Calculate the Size needeed for the index itself
    uint32_t serializedSize = 0;
    if (additionalInfo != NULL) {
      serializedSize += additionalInfo->getSerializedSize();
    }
    // Size of current iterator type
    Index<T> tempIndex;
    serializedSize += tempIndex.getSerializedSize();

    // Add Size of Array
    serializedSize += sizeof(uint32_t);                                  // size of array
    serializedSize += (tempIndex.getSerializedSize() * maxNrOfIndices);  // size of elements
    serializedSize += sizeof(uint16_t);                                  // size of crc

    // Calculate new size after index
    if (serializedSize > totalSize) {
      error << "IndexedRingMemory: Store is too small for index" << std::endl;
    }
    uint32_t useableSize = totalSize - serializedSize;
    // Update the totalSize for calculations
    totalSize = useableSize;

    // True StartAddress
    uint32_t trueStartAddress = indexAddress + serializedSize;

    // Calculate True number of Blocks and reset size of true Number of Blocks
    uint32_t trueNumberOfBlocks =
        floor(static_cast<double>(totalSize) / static_cast<double>(bytesPerBlock));

    // allocate memory now
    this->entries = new Index<T>[trueNumberOfBlocks];
    this->size = trueNumberOfBlocks;
    this->maxSize_ = trueNumberOfBlocks;
    this->allocated = true;

    // Check trueNumberOfBlocks
    if (trueNumberOfBlocks < 1) {
      error << "IndexedRingMemory: Invalid Number of Blocks: " << trueNumberOfBlocks;
    }

    // Fill address into index
    uint32_t address = trueStartAddress;
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      it->setBlockStartAddress(address);
      it->setSize(0);
      it->setStoredPackets(0);
      address += bytesPerBlock;
    }

    // Initialize iterators
    currentWriteBlock = this->begin();
    currentReadBlock = this->begin();
    lastBlockToRead = this->begin();

    // Check last blockSize
    uint32_t lastBlockSize =
        (trueStartAddress + useableSize) - (this->back()->getBlockStartAddress());
    if ((lastBlockSize < bytesPerBlock) && (this->size > 1)) {
      // remove the last Block so the second last block has more size
      this->size -= 1;
      debug << "IndexedRingMemory: Last Block is smaller than bytesPerBlock, removed last block"
            << std::endl;
    }
  }

  /**
   * Resets the whole index, the iterators and executes the given reset function on every index type
   * @param typeResetFnc static reset function which accepts a pointer to the index Type
   */
  void reset(void (*typeResetFnc)(T*)) {
    currentReadBlock = this->begin();
    currentWriteBlock = this->begin();
    lastBlockToRead = this->begin();
    currentReadSize = 0;
    currentReadBlockSizeCached = 0;
    lastBlockToReadSize = 0;
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      it->setSize(0);
      it->setStoredPackets(0);
      (*typeResetFnc)(it->modifyIndexType());
    }
  }

  void resetBlock(typename IndexedRingMemoryArray<T>::Iterator it, void (*typeResetFnc)(T*)) {
    it->setSize(0);
    it->setStoredPackets(0);
    (*typeResetFnc)(it->modifyIndexType());
  }

  /*
   * Reading
   */

  void setCurrentReadBlock(typename IndexedRingMemoryArray<T>::Iterator it) {
    currentReadBlock = it;
    currentReadBlockSizeCached = it->getSize();
  }

  void resetRead() {
    currentReadBlock = this->begin();
    currentReadSize = 0;
    currentReadBlockSizeCached = this->begin()->getSize();
    lastBlockToRead = currentWriteBlock;
    lastBlockToReadSize = currentWriteBlock->getSize();
  }
  /**
   * Sets the last block to read to this iterator.
   * Can be used to dump until block x
   * @param it The iterator for the last read block
   */
  void setLastBlockToRead(typename IndexedRingMemoryArray<T>::Iterator it) {
    lastBlockToRead = it;
    lastBlockToReadSize = it->getSize();
  }

  /**
   * Set the read pointer to the first written Block, which is the first non empty block in front of
   * the write block Can be the currentWriteBlock as well
   */
  void readOldest() {
    resetRead();
    currentReadBlock = getNextNonEmptyBlock();
    currentReadBlockSizeCached = currentReadBlock->getSize();
  }

  /**
   * Sets the current read iterator to the next Block and resets the current read size
   * The current size of the block will be cached to avoid race condition between write and read
   * If the end of the ring is reached the read pointer will be set to the begin
   */
  void readNext() {
    currentReadSize = 0;
    if ((this->size != 0) && (currentReadBlock.value == this->back())) {
      currentReadBlock = this->begin();
    } else {
      currentReadBlock++;
    }

    currentReadBlockSizeCached = currentReadBlock->getSize();
  }

  /**
   * Returns the address which is currently read from
   * @return Address to read from
   */
  uint32_t getCurrentReadAddress() const {
    return getAddressOfCurrentReadBlock() + currentReadSize;
  }
  /**
   * Adds readSize to the current size and checks if the read has no more data left and advances the
   * read block
   * @param readSize The size that was read
   * @return Returns true if the read can go on
   */
  bool addReadSize(uint32_t readSize) {
    if (currentReadBlock == lastBlockToRead) {
      // The current read block is the last to read
      if ((currentReadSize + readSize) < lastBlockToReadSize) {
        // the block has more data -> return true
        currentReadSize += readSize;
        return true;
      } else {
        // Reached end of read -> return false
        currentReadSize = lastBlockToReadSize;
        return false;
      }
    } else {
      // We are not in the last Block
      if ((currentReadSize + readSize) < currentReadBlockSizeCached) {
        // The current Block has more data
        currentReadSize += readSize;
        return true;
      } else {
        // The current block is written completely
        readNext();
        if (currentReadBlockSizeCached == 0) {
          // Next block is empty
          typename IndexedRingMemoryArray<T>::Iterator it(currentReadBlock);
          // Search if any block between this and the last block is not empty
          for (; it != lastBlockToRead; ++it) {
            if (it == this->end()) {
              // This is the end, next block is the begin
              it = this->begin();
              if (it == lastBlockToRead) {
                // Break if the begin is the lastBlockToRead
                break;
              }
            }
            if (it->getSize() != 0) {
              // This is a non empty block. Go on reading with this block
              currentReadBlock = it;
              currentReadBlockSizeCached = it->getSize();
              return true;
            }
          }
          // reached lastBlockToRead and every block was empty, check if the last block is also
          // empty
          if (lastBlockToReadSize != 0) {
            // go on with last Block
            currentReadBlock = lastBlockToRead;
            currentReadBlockSizeCached = lastBlockToReadSize;
            return true;
          }
          // There is no non empty block left
          return false;
        }
        // Size is larger than 0
        return true;
      }
    }
  }
  uint32_t getRemainigSizeOfCurrentReadBlock() const {
    if (currentReadBlock == lastBlockToRead) {
      return (lastBlockToReadSize - currentReadSize);
    } else {
      return (currentReadBlockSizeCached - currentReadSize);
    }
  }

  uint32_t getAddressOfCurrentReadBlock() const { return currentReadBlock->getBlockStartAddress(); }

  /**
   * Gets the next non empty Block after the current write block,
   * @return Returns the iterator to the block. If there is non, the current write block is returned
   */
  typename IndexedRingMemoryArray<T>::Iterator getNextNonEmptyBlock() const {
    for (typename IndexedRingMemoryArray<T>::Iterator it = getNextWrite(); it != currentWriteBlock;
         ++it) {
      if (it == this->end()) {
        it = this->begin();
        if (it == currentWriteBlock) {
          break;
        }
      }
      if (it->getSize() != 0) {
        return it;
      }
    }
    return currentWriteBlock;
  }

  /**
   * Returns a copy of the oldest Index type
   * @return Type of Index
   */
  T* getOldest() { return (getNextNonEmptyBlock()->modifyIndexType()); }

  /*
   * Writing
   */
  uint32_t getAddressOfCurrentWriteBlock() const {
    return currentWriteBlock->getBlockStartAddress();
  }

  uint32_t getSizeOfCurrentWriteBlock() const { return currentWriteBlock->getSize(); }

  uint32_t getCurrentWriteAddress() const {
    return getAddressOfCurrentWriteBlock() + getSizeOfCurrentWriteBlock();
  }

  void clearCurrentWriteBlock() {
    currentWriteBlock->setSize(0);
    currentWriteBlock->setStoredPackets(0);
  }

  void addCurrentWriteBlock(uint32_t size, uint32_t storedPackets) {
    currentWriteBlock->addSize(size);
    currentWriteBlock->addStoredPackets(storedPackets);
  }

  T* modifyCurrentWriteBlockIndexType() { return currentWriteBlock->modifyIndexType(); }
  void updatePreviousWriteSize(uint32_t size, uint32_t storedPackets) {
    typename IndexedRingMemoryArray<T>::Iterator it = getPreviousBlock(currentWriteBlock);
    it->addSize(size);
    it->addStoredPackets(storedPackets);
  }

  /**
   * Checks if the block has enough space for sizeToWrite
   * @param sizeToWrite The data to be written in the Block
   * @return Returns true if size to write is smaller the remaining size of the block
   */
  bool hasCurrentWriteBlockEnoughSpace(uint32_t sizeToWrite) {
    typename IndexedRingMemoryArray<T>::Iterator next = getNextWrite();
    uint32_t addressOfNextBlock = next->getBlockStartAddress();
    uint32_t availableSize = ((addressOfNextBlock + totalSize) -
                              (getAddressOfCurrentWriteBlock() + getSizeOfCurrentWriteBlock())) %
                             totalSize;
    return (sizeToWrite < availableSize);
  }

  /**
   * Checks if the store is full if overwrite old is false
   * @return Returns true if it is writeable and false if not
   */
  bool isNextBlockWritable() {
    // First check if this is the end of the list
    typename IndexedRingMemoryArray<T>::Iterator next;
    next = getNextWrite();
    if ((next->getSize() != 0) && (!overwriteOld)) {
      return false;
    }
    return true;
  }

  /**
   * Updates current write Block Index Type
   * @param infoOfNewBlock
   */
  void updateCurrentBlock(T* infoOfNewBlock) { currentWriteBlock->setIndexType(infoOfNewBlock); }

  /**
   * Succeed to next block, returns FAILED if overwrite is false and the store is full
   * @return
   */
  ReturnValue_t writeNext() {
    // Check Next Block
    if (!isNextBlockWritable()) {
      // The Index is full and does not overwrite old
      return returnvalue::FAILED;
    }
    // Next block can be written, update Metadata
    currentWriteBlock = getNextWrite();
    currentWriteBlock->setSize(0);
    currentWriteBlock->setStoredPackets(0);
    return returnvalue::OK;
  }

  /**
   * Serializes the Index and calculates the CRC.
   * Parameters according to HasSerializeIF
   * @param buffer
   * @param size
   * @param maxSize
   * @param streamEndianness
   * @return
   */
  ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                          Endianness streamEndianness) const {
    uint8_t* crcBuffer = *buffer;
    uint32_t oldSize = *size;
    if (additionalInfo != NULL) {
      additionalInfo->serialize(buffer, size, maxSize, streamEndianness);
    }
    ReturnValue_t result = currentWriteBlock->serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    result = SerializeAdapter::serialize(&this->size, buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }

    uint32_t i = 0;
    while ((result == returnvalue::OK) && (i < this->size)) {
      result =
          SerializeAdapter::serialize(&this->entries[i], buffer, size, maxSize, streamEndianness);
      ++i;
    }
    if (result != returnvalue::OK) {
      return result;
    }
    uint16_t crc = Calculate_CRC(crcBuffer, (*size - oldSize));
    result = SerializeAdapter::serialize(&crc, buffer, size, maxSize, streamEndianness);
    return result;
  }

  /**
   * Get the serialized Size of the index
   * @return The serialized size of the index
   */
  size_t getSerializedSize() const {
    uint32_t size = 0;
    if (additionalInfo != NULL) {
      size += additionalInfo->getSerializedSize();
    }
    size += currentWriteBlock->getSerializedSize();
    size += SerializeAdapter::getSerializedSize(&this->size);
    size += (this->entries[0].getSerializedSize()) * this->size;
    uint16_t crc = 0;
    size += SerializeAdapter::getSerializedSize(&crc);
    return size;
  }
  /**
   * DeSerialize the Indexed Ring from a buffer, deSerializes the current write iterator
   * CRC Has to be checked before!
   * @param buffer
   * @param size
   * @param streamEndianness
   * @return
   */

  ReturnValue_t deSerialize(const uint8_t** buffer, size_t* size, Endianness streamEndianness) {
    ReturnValue_t result = returnvalue::OK;
    if (additionalInfo != NULL) {
      result = additionalInfo->deSerialize(buffer, size, streamEndianness);
    }
    if (result != returnvalue::OK) {
      return result;
    }

    Index<T> tempIndex;
    result = tempIndex.deSerialize(buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    uint32_t tempSize = 0;
    result = SerializeAdapter::deSerialize(&tempSize, buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
    if (this->size != tempSize) {
      return returnvalue::FAILED;
    }
    uint32_t i = 0;
    while ((result == returnvalue::OK) && (i < this->size)) {
      result = SerializeAdapter::deSerialize(&this->entries[i], buffer, size, streamEndianness);
      ++i;
    }
    if (result != returnvalue::OK) {
      return result;
    }
    typename IndexedRingMemoryArray<T>::Iterator cmp(&tempIndex);
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      if (*(cmp.value) == *(it.value)) {
        currentWriteBlock = it;
        return returnvalue::OK;
      }
    }
    // Reached if current write block iterator is not found
    return returnvalue::FAILED;
  }

  uint32_t getIndexAddress() const { return indexAddress; }

  /*
   * Statistics
   */
  uint32_t getStoredPackets() const {
    uint32_t size = 0;
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      size += it->getStoredPackets();
    }
    return size;
  }

  uint32_t getTotalSize() const { return totalSize; }

  uint32_t getCurrentSize() const {
    uint32_t size = 0;
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      size += it->getSize();
    }
    return size;
  }

  bool isEmpty() const { return getCurrentSize() == 0; }

  double getPercentageFilled() const {
    uint32_t filledSize = 0;
    for (typename IndexedRingMemoryArray<T>::Iterator it = this->begin(); it != this->end(); ++it) {
      filledSize += it->getSize();
    }

    return (double)filledSize / (double)this->totalSize;
  }

  typename IndexedRingMemoryArray<T>::Iterator getCurrentWriteBlock() const {
    return currentWriteBlock;
  }
  /**
   * Get the next block of the currentWriteBlock.
   * Returns the first one if currentWriteBlock is the last one
   * @return Iterator pointing to the next block after currentWriteBlock
   */
  typename IndexedRingMemoryArray<T>::Iterator getNextWrite() const {
    typename IndexedRingMemoryArray<T>::Iterator next(currentWriteBlock);
    if ((this->size != 0) && (currentWriteBlock.value == this->back())) {
      next = this->begin();
    } else {
      ++next;
    }
    return next;
  }
  /**
   * Get the block in front of the Iterator
   * Returns the last block if it is the first block
   * @param it iterator which you want the previous block from
   * @return pointing to the block before it
   */
  typename IndexedRingMemoryArray<T>::Iterator getPreviousBlock(
      typename IndexedRingMemoryArray<T>::Iterator it) {
    if (this->begin() == it) {
      typename IndexedRingMemoryArray<T>::Iterator next((this->back()));
      return next;
    }
    typename IndexedRingMemoryArray<T>::Iterator next(it);
    --next;
    return next;
  }

 private:
  // The total size used by the blocks (without index)
  uint32_t totalSize;

  // The address of the index
  const uint32_t indexAddress;

  // The iterators for writing and reading
  typename IndexedRingMemoryArray<T>::Iterator currentWriteBlock;
  typename IndexedRingMemoryArray<T>::Iterator currentReadBlock;

  // How much of the current read block is read already
  uint32_t currentReadSize;

  // Cached Size of current read block
  uint32_t currentReadBlockSizeCached;

  // Last block of current write (should be write block)
  typename IndexedRingMemoryArray<T>::Iterator lastBlockToRead;
  // current size of last Block to read
  uint32_t lastBlockToReadSize;

  // Additional Info to be serialized with the index
  SerializeIF* additionalInfo;

  // Does it overwrite old blocks?
  const bool overwriteOld;
};

#endif /* FRAMEWORK_CONTAINER_INDEXEDRINGMEMORY_H_ */
