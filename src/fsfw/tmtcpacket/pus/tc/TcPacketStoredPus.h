#ifndef FSFW_TMTCPACKET_PUS_TCPACKETSTOREDPUSA_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETSTOREDPUSA_H_

#include "TcPacketStoredBase.h"
#include "TcPacketPus.h"

class TcPacketStoredPus:
        public TcPacketStoredBase,
        public TcPacketPus {
public:
    /**
     * With this constructor, new space is allocated in the packet store and
     * a new PUS Telecommand Packet is created there.
     * Packet Application Data passed in data is copied into the packet.
     * @param apid          Sets the packet's APID field.
     * @param service       Sets the packet's Service ID field.
     *                      This specifies the destination service.
     * @param subservice    Sets the packet's Service Subtype field.
     *                      This specifies the destination sub-service.
     * @param sequence_count Sets the packet's Source Sequence Count field.
     * @param data      The data to be copied to the Application Data Field.
     * @param size      The amount of data to be copied.
     * @param ack           Set's the packet's Ack field, which specifies
     *                      number of verification packets returned
     *                      for this command.
     */
    TcPacketStoredPus(uint16_t apid, uint8_t service, uint8_t subservice,
            uint8_t sequence_count = 0, const uint8_t* data = nullptr,
            size_t size = 0, uint8_t ack = TcPacketPusBase::ACK_ALL);
    /**
     * Create stored packet with existing data.
     * @param data
     * @param size
     */
    TcPacketStoredPus(const uint8_t* data, size_t size);
    /**
     * Create stored packet from existing packet in store
     * @param setAddress
     */
    TcPacketStoredPus(store_address_t setAddress);
    TcPacketStoredPus();

    ReturnValue_t deletePacket() override;
    TcPacketPusBase* getPacketBase();

private:

    bool isSizeCorrect() override;
};



#endif /* FSFW_TMTCPACKET_PUS_TCPACKETSTOREDPUSA_H_ */
