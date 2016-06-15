
#include <framework/tmtcpacket/pus/TmPacketMinimal.h>

TmPacketMinimal::TmPacketMinimal(const uint8_t* set_data) : SpacePacketBase( set_data ) {
	this->tm_data = (TmPacketMinimalPointer*)set_data;
}

TmPacketMinimal::~TmPacketMinimal() {
}

uint8_t TmPacketMinimal::getService() {
	return tm_data->data_field.service_type;
}

uint8_t TmPacketMinimal::getSubService() {
	return tm_data->data_field.service_subtype;
}

uint8_t TmPacketMinimal::getPacketSubcounter() {
	return tm_data->data_field.subcounter;
}
