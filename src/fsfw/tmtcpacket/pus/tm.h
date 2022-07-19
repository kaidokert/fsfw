#ifndef FSFW_TMTCPACKET_PUS_TM_H_
#define FSFW_TMTCPACKET_PUS_TM_H_

#include "fsfw/FSFW.h"

#if FSFW_USE_PUS_C_TELEMETRY == 1
#include "tm/TmPacketPusC.h"
#include "tm/TmPacketStoredPusC.h"
#else
#include "tm/TmPacketPusA.h"
#include "tm/TmPacketStoredPusA.h"
#endif

#include "tm/PusTmMinimal.h"

#endif /* FSFW_TMTCPACKET_PUS_TM_H_ */
