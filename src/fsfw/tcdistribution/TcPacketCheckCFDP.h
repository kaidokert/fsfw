#ifndef FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_
#define FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_

#include "TcPacketCheckIF.h"

#include "fsfw/FSFW.h"

class CFDPPacketStored;

/**
 * This class performs a formal packet check for incoming CFDP Packets.
 * @ingroup tc_distribution
 */
class TcPacketCheckCFDP :
		public TcPacketCheckIF,
		public HasReturnvaluesIF {
protected:
    /**
     * The packet id each correct packet should have.
     * It is composed of the APID and some static fields.
     */
    uint16_t apid;
public:
    /**
     * The constructor only sets the APID attribute.
     * @param set_apid The APID to set.
     */
    TcPacketCheckCFDP(uint16_t setApid);

    ReturnValue_t checkPacket(SpacePacketBase* currentPacket) override;

    uint16_t getApid() const;
};

#endif /* FSFW_TCDISTRIBUTION_TCPACKETCHECKCFDP_H_ */
