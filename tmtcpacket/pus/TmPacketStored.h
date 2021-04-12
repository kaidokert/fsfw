#ifndef FSFW_TMTCPACKET_PUS_TMPACKETSTORED_H_
#define FSFW_TMTCPACKET_PUS_TMPACKETSTORED_H_

#include <FSFWConfig.h>

#if FSFW_USE_PUS_C_TELEMETRY == 1
#include "TmPacketStoredPusC.h"
#else
#include "TmPacketStoredPusA.h"
#endif


#endif /* FSFW_TMTCPACKET_PUS_TMPACKETSTORED_H_ */
