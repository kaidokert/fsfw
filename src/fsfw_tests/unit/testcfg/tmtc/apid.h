#ifndef CONFIG_TMTC_APID_H_
#define CONFIG_TMTC_APID_H_

#include <cstdint>

/**
 * Application Process Definition: entity, uniquely identified by an
 * application process ID (APID), capable of generating telemetry source
 * packets and receiving telecommand packets.
 *
 * Chose APID(s) for mission and define it here.
 */
namespace apid {
static const uint16_t DEFAULT_APID = 0x00;
}

#endif /* CONFIG_TMTC_APID_H_ */
