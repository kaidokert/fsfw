#include "fsfw/datalinklayer/DataLinkLayer.h"

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

DataLinkLayer::DataLinkLayer(uint8_t* set_frame_buffer, ClcwIF* setClcw,
                             uint8_t set_start_sequence_length, uint16_t set_scid)
    : spacecraftId(set_scid),
      frameBuffer(set_frame_buffer),
      clcw(setClcw),
      receivedDataLength(0),
      currentFrame(NULL),
      startSequenceLength(set_start_sequence_length) {
  // Nothing to do except from setting the values above.
}

DataLinkLayer::~DataLinkLayer() {}

ReturnValue_t DataLinkLayer::frameDelimitingAndFillRemoval() {
  if ((receivedDataLength - startSequenceLength) < FRAME_PRIMARY_HEADER_LENGTH) {
    return SHORTER_THAN_HEADER;
  }
  // Removing start sequence.
  // SHOULDDO: Not implemented here.
  while (*frameBuffer == START_SEQUENCE_PATTERN) {
    frameBuffer++;
  }
  TcTransferFrame frame_candidate(frameBuffer);
  this->currentFrame = frame_candidate;  // should work with shallow copy.

  return returnvalue::OK;
}

ReturnValue_t DataLinkLayer::frameValidationCheck() {
  // Check TF_version number
  if (this->currentFrame.getVersionNumber() != FRAME_VERSION_NUMBER_DEFAULT) {
    return WRONG_TF_VERSION;
  }
  // Check SpaceCraft ID
  if (this->currentFrame.getSpacecraftId() != this->spacecraftId) {
    return WRONG_SPACECRAFT_ID;
  }
  // Check other header limitations:
  if (!this->currentFrame.bypassFlagSet() && this->currentFrame.controlCommandFlagSet()) {
    return NO_VALID_FRAME_TYPE;
  }
  //- Spares are zero
  if (!this->currentFrame.spareIsZero()) {
    return NO_VALID_FRAME_TYPE;
  }
  // Compare detected frame length with the one in the header
  uint16_t length = currentFrame.getFullSize();
  if (length > receivedDataLength) {
    // Frame is too long or just right
    //		error << "frameValidationCheck: Too short.";
    //		currentFrame.print();
    return TOO_SHORT;
  }
  if (USE_CRC) {
    return this->frameCheckCRC();
  }
  return returnvalue::OK;
}

ReturnValue_t DataLinkLayer::frameCheckCRC() {
  uint16_t checkValue =
      CRC::crc16ccitt(this->currentFrame.getFullFrame(), this->currentFrame.getFullSize());
  if (checkValue == 0) {
    return returnvalue::OK;
  } else {
    return CRC_FAILED;
  }
}

ReturnValue_t DataLinkLayer::allFramesReception() {
  ReturnValue_t status = this->frameDelimitingAndFillRemoval();
  if (status != returnvalue::OK) {
    return status;
  }
  return this->frameValidationCheck();
}

ReturnValue_t DataLinkLayer::masterChannelDemultiplexing() {
  // Nothing to do at present. Ideally, there would be a map of MCID's identifying which MC to use.
  return virtualChannelDemultiplexing();
}

ReturnValue_t DataLinkLayer::virtualChannelDemultiplexing() {
  uint8_t vcId = currentFrame.getVirtualChannelId();
  virtualChannelIterator iter = virtualChannels.find(vcId);
  if (iter == virtualChannels.end()) {
    // Do not report because passive board will get this error all the time.
    return returnvalue::OK;
  } else {
    return (iter->second)->frameAcceptanceAndReportingMechanism(&currentFrame, clcw);
  }
}

ReturnValue_t DataLinkLayer::processFrame(uint16_t length) {
  receivedDataLength = length;
  ReturnValue_t status = allFramesReception();
  if (status != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DataLinkLayer::processFrame: frame reception failed. "
                  "Error code: "
               << std::hex << status << std::dec << std::endl;
#endif
    //		currentFrame.print();
    return status;
  } else {
    return masterChannelDemultiplexing();
  }
}

ReturnValue_t DataLinkLayer::addVirtualChannel(uint8_t virtualChannelId,
                                               VirtualChannelReceptionIF* object) {
  std::pair<virtualChannelIterator, bool> returnValue = virtualChannels.insert(
      std::pair<uint8_t, VirtualChannelReceptionIF*>(virtualChannelId, object));
  if (returnValue.second == true) {
    return returnvalue::OK;
  } else {
    return returnvalue::FAILED;
  }
}

ReturnValue_t DataLinkLayer::initialize() {
  ReturnValue_t returnValue = returnvalue::FAILED;
  // Set Virtual Channel ID to first virtual channel instance in this DataLinkLayer instance to
  // avoid faulty information (e.g. 0) in the VCID.
  if (virtualChannels.begin() != virtualChannels.end()) {
    clcw->setVirtualChannel(virtualChannels.begin()->second->getChannelId());
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "DataLinkLayer::initialize: No VC assigned to this DLL instance! " << std::endl;
#endif
    return returnvalue::FAILED;
  }

  for (virtualChannelIterator iterator = virtualChannels.begin(); iterator != virtualChannels.end();
       iterator++) {
    returnValue = iterator->second->initialize();
    if (returnValue != returnvalue::OK) break;
  }
  return returnValue;
}
