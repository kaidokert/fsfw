#ifndef FSFW_TMTCPACKET_PUS_TM_H_
#define FSFW_TMTCPACKET_PUS_TM_H_

#include "../../FSFW.h"

#if FSFW_USE_PUS_C_TELEMETRY == 1
#include "tm/TmPacketPusC.h"
#include "tm/TmPacketStoredPusC.h"
#else
#include "tm/TmPacketPusA.h"
#include "tm/TmPacketStoredPusA.h"
#endif

#include "tm/TmPacketMinimal.h"

#endif /* FSFW_TMTCPACKET_PUS_TM_H_ */
