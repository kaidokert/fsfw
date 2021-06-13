#ifndef FSFW_TMTCPACKET_PUS_TCPACKETSTORED_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETSTORED_H_

#include <FSFWConfig.h>

#if FSFW_USE_PUS_C_TELECOMMANDS == 1
#include "TcPacketStoredPusC.h"
#else
#include "TcPacketStoredPusA.h"
#endif


#endif /* FSFW_TMTCPACKET_PUS_TCPACKETSTORED_H_ */
