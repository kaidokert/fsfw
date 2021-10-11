#ifndef FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_
#define FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_

#include <cstdint>

namespace pus {

//! Version numbers according to ECSS-E-ST-70-41C p.439
enum PusVersion: uint8_t {
    PUS_A_VERSION = 1,
    PUS_C_VERSION = 2
};

}

#endif /* FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_ */
