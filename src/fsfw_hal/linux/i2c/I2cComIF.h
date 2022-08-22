#ifndef LINUX_I2C_I2COMIF_H_
#define LINUX_I2C_I2COMIF_H_

#include <fsfw/devicehandlers/DeviceCommunicationIF.h>
#include <fsfw/objectmanager/SystemObject.h>

#include <unordered_map>
#include <vector>

#include "I2cCookie.h"

/**
 * @brief 	This is the communication interface for I2C devices connected
 * 			to a system running a Linux OS.
 *
 * @note    The Xilinx Linux kernel might not support to read more than 255 bytes at once.
 *
 * @author 	J. Meier
 */
class I2cComIF : public DeviceCommunicationIF, public SystemObject {
 public:
  I2cComIF(object_id_t objectId);

  virtual ~I2cComIF();

  ReturnValue_t initializeInterface(CookieIF *cookie) override;
  ReturnValue_t sendMessage(CookieIF *cookie, const uint8_t *sendData, size_t sendLen) override;
  ReturnValue_t getSendSuccess(CookieIF *cookie) override;
  ReturnValue_t requestReceiveMessage(CookieIF *cookie, size_t requestLen) override;
  ReturnValue_t readReceivedMessage(CookieIF *cookie, uint8_t **buffer, size_t *size) override;

 private:
  struct I2cInstance {
    std::vector<uint8_t> replyBuffer;
    size_t replyLen;
  };

  using I2cDeviceMap = std::unordered_map<address_t, I2cInstance>;
  using I2cDeviceMapIter = I2cDeviceMap::iterator;

  /* In this map all i2c devices will be registered with their address and
   * the appropriate file descriptor will be stored */
  I2cDeviceMap i2cDeviceMap;
  I2cDeviceMapIter i2cDeviceMapIter;

  /**
   * @brief	This function opens an I2C device and binds the opened file
   * 			to a specific I2C address.
   * @param deviceFile	The name of the device file. E.g. i2c-0
   * @param i2cAddress	The address of the i2c slave device.
   * @param fileDescriptor	Pointer to device descriptor.
   * @return	returnvalue::OK if successful, otherwise returnvalue::FAILED.
   */
  ReturnValue_t openDevice(std::string deviceFile, address_t i2cAddress, int *fileDescriptor);
};

#endif /* LINUX_I2C_I2COMIF_H_ */
