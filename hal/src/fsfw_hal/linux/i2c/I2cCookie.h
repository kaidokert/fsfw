#ifndef LINUX_I2C_I2CCOOKIE_H_
#define LINUX_I2C_I2CCOOKIE_H_

#include <fsfw/devicehandlers/CookieIF.h>

#include <string>

/**
 * @brief   Cookie for the i2cDeviceComIF.
 *
 * @author 	J. Meier
 */
class I2cCookie : public CookieIF {
 public:
  /**
   * @brief	Constructor for the I2C cookie.
   * @param i2cAddress_	The i2c address of the target device.
   * @param maxReplyLen_	The maximum expected length of a reply from the
   * 						target device.
   * @param devicFile_    The device file specifying the i2c interface to use. E.g. "/dev/i2c-0".
   */
  I2cCookie(address_t i2cAddress_, size_t maxReplyLen_, std::string deviceFile_);

  virtual ~I2cCookie();

  address_t getAddress() const;
  size_t getMaxReplyLen() const;
  std::string getDeviceFile() const;

 private:
  address_t i2cAddress = 0;
  size_t maxReplyLen = 0;
  std::string deviceFile;
};

#endif /* LINUX_I2C_I2CCOOKIE_H_ */
