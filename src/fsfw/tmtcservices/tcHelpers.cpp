#include "tcHelpers.h"

ReturnValue_t tc::prepareTcReader(StorageManagerIF &tcStore, store_address_t storeId,
                                  PusTcReader &tcReader) {
  const uint8_t *dataPtr;
  size_t dataLen = 0;
  ReturnValue_t result = tcStore.getData(storeId, &dataPtr, &dataLen);
  if (result != returnvalue::OK) {
    return result;
  }
  result = tcReader.setReadOnlyData(dataPtr, dataLen);
  if (result != returnvalue::OK) {
    return result;
  }
  return tcReader.parseDataWithoutCrcCheck();
}
