#ifndef FSFW_RMAP_RMAPCOOKIE_H_
#define FSFW_RMAP_RMAPCOOKIE_H_

#include <cstddef>

#include "fsfw/devicehandlers/CookieIF.h"
#include "rmapConf.h"
#include "rmapStructs.h"

class RMAPChannelIF;

class RMAPCookie : public CookieIF {
 public:
  // To Uli: Sorry, I need an empty ctor to initialize an array of cookies.
  RMAPCookie();

  RMAPCookie(uint32_t set_address, uint8_t set_extended_address, RMAPChannelIF *set_channel,
             uint8_t set_command_mask, size_t maxReplyLen = 0);
  virtual ~RMAPCookie();

  void setAddress(uint32_t address);
  uint32_t getAddress();

  void setExtendedAddress(uint8_t);
  uint8_t getExtendedAddress();

  void setChannel(RMAPChannelIF *channel);
  RMAPChannelIF *getChannel();

  void setCommandMask(uint8_t commandMask);
  uint8_t getCommandMask();

  size_t getMaxReplyLen() const;
  void setMaxReplyLen(size_t maxReplyLen);

  uint16_t getTransactionIdentifier() const;
  void setTransactionIdentifier(uint16_t id_);

  RMAPStructs::rmap_cmd_header *getHeader();

  uint32_t getDataLength() const;
  void setDataLength(uint32_t lenght_);

  uint8_t getDataCrc() const { return dataCRC; }

  void setDataCrc(uint8_t dataCrc) { dataCRC = dataCrc; }

 protected:
  RMAPStructs::rmap_cmd_header header;
  RMAPChannelIF *channel;
  uint8_t command_mask;
  uint32_t maxReplyLen;
  uint8_t dataCRC;
};

#endif /* FSFW_RMAP_RMAPCOOKIE_H_ */
