#ifndef RMAPCOOKIE_H_
#define RMAPCOOKIE_H_

#include <framework/devicehandlers/Cookie.h>
#include <framework/rmap/rmapStructs.h>

class RMAPChannelIF;

class RMAPCookie : public Cookie{
	friend class RMAP;
	friend class RmapSPWChannel;
public:
	//To Uli: Sorry, I need an empty ctor to initialize an array of cookies.
	RMAPCookie();

	RMAPCookie(uint32_t set_address, uint8_t set_extended_address,
			RMAPChannelIF *set_channel, uint8_t set_command_mask, uint32_t maxReplyLen = 0);
	virtual ~RMAPCookie();


	void setAddress(uint32_t address);
	uint32_t getAddress();
	void setExtendedAddress(uint8_t);
	uint8_t getExtendedAddress();
	void setChannel(RMAPChannelIF *channel);
	RMAPChannelIF *getChannel();
	void setCommandMask(uint8_t commandMask);
	uint8_t getCommandMask();
	uint32_t getMaxReplyLen() const;
	void setMaxReplyLen(uint32_t maxReplyLen);

	//rmap_cookie* getDeviceDescriptor();

protected:
	RMAPStructs::rmap_cmd_header header;
	void *txdesc;
	uint8_t rxdesc_index;
	RMAPChannelIF *channel;
	uint8_t command_mask;
	uint32_t maxReplyLen;
};

#endif /* RMAPCOOKIE_H_ */
