#ifndef CCSDS_HEADER_H_
#define CCSDS_HEADER_H_

#include <stdint.h>

struct CCSDSPrimaryHeader {
	uint8_t packet_id_h;
	uint8_t packet_id_l;
	uint8_t sequence_control_h;
	uint8_t sequence_control_l;
	uint8_t packet_length_h;
	uint8_t packet_length_l;
};

#endif /* CCSDS_HEADER_H_ */
