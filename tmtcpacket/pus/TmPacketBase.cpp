#include "TmPacketBase.h"

#include "../../globalfunctions/CRC.h"
#include "../../globalfunctions/arrayprinter.h"
#include "../../objectmanager/ObjectManager.h"
#include "../../serviceinterface/ServiceInterface.h"
#include "../../timemanager/CCSDSTime.h"

#include <cstring>

TimeStamperIF* TmPacketBase::timeStamper = nullptr;
object_id_t TmPacketBase::timeStamperId = objects::NO_OBJECT;

TmPacketBase::TmPacketBase(uint8_t* setData):
        SpacePacketBase(setData) {
}

TmPacketBase::~TmPacketBase() {
    //Nothing to do.
}


uint16_t TmPacketBase::getSourceDataSize() {
    return getPacketDataLength() - getDataFieldSize() - CRC_SIZE + 1;
}

uint16_t TmPacketBase::getErrorControl() {
    uint32_t size = getSourceDataSize() + CRC_SIZE;
    uint8_t* p_to_buffer = getSourceData();
    return (p_to_buffer[size - 2] << 8) + p_to_buffer[size - 1];
}

void TmPacketBase::setErrorControl() {
    uint32_t full_size = getFullSize();
    uint16_t crc = CRC::crc16ccitt(getWholeData(), full_size - CRC_SIZE);
    uint32_t size = getSourceDataSize();
    getSourceData()[size] = (crc & 0XFF00) >> 8;	// CRCH
    getSourceData()[size + 1] = (crc) & 0X00FF; 		// CRCL
}

ReturnValue_t TmPacketBase::getPacketTime(timeval* timestamp) const {
    size_t tempSize = 0;
    return CCSDSTime::convertFromCcsds(timestamp, getPacketTimeRaw(),
            &tempSize, getTimestampSize());
}

void TmPacketBase::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "TmPacketBase::print: " << std::endl;
#endif
    arrayprinter::print(getWholeData(), getFullSize());
}

bool TmPacketBase::checkAndSetStamper() {
    if (timeStamper == NULL) {
        timeStamper = ObjectManager::instance()->get<TimeStamperIF>(timeStamperId);
        if (timeStamper == NULL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::warning << "TmPacketBase::checkAndSetStamper: Stamper not found!" << std::endl;
#else
            sif::printWarning("TmPacketBase::checkAndSetStamper: Stamper not found!\n");
#endif
            return false;
        }
    }
    return true;
}

