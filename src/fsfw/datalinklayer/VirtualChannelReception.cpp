/**
 * @file	VirtualChannelReception.cpp
 * @brief	This file defines the VirtualChannelReception class.
 * @date	26.03.2013
 * @author	baetz
 */

#include "fsfw/datalinklayer/VirtualChannelReception.h"

#include "fsfw/datalinklayer/BCFrame.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

VirtualChannelReception::VirtualChannelReception(uint8_t setChannelId,
                                                 uint8_t setSlidingWindowWidth)
    : channelId(setChannelId),
      slidingWindowWidth(setSlidingWindowWidth),
      positiveWindow(setSlidingWindowWidth / 2),
      negativeWindow(setSlidingWindowWidth / 2),
      currentState(&openState),
      openState(this),
      waitState(this),
      lockoutState(this),
      vR(0),
      farmBCounter(0) {
  internalClcw.setVirtualChannel(channelId);
}

ReturnValue_t VirtualChannelReception::mapDemultiplexing(TcTransferFrame* frame) {
  uint8_t mapId = frame->getMAPId();
  mapChannelIterator iter = mapChannels.find(mapId);
  if (iter == mapChannels.end()) {
    //		error << "VirtualChannelReception::mapDemultiplexing on VC " << std::hex << (int)
    // channelId
    //				<< ": MapChannel " << (int) mapId << std::dec << " not found." <<
    // std::endl;
    return VC_NOT_FOUND;
  } else {
    return (iter->second)->extractPackets(frame);
  }
}

ReturnValue_t VirtualChannelReception::doFARM(TcTransferFrame* frame, ClcwIF* clcw) {
  uint8_t bypass = frame->bypassFlagSet();
  uint8_t controlCommand = frame->controlCommandFlagSet();
  uint8_t typeValue = (bypass << 1) + controlCommand;
  switch (typeValue) {
    case AD_FRAME:
      return currentState->handleADFrame(frame, clcw);
    case BD_FRAME:
      return handleBDFrame(frame, clcw);
    case BC_FRAME:
      return handleBCFrame(frame, clcw);
    default:
      return ILLEGAL_FLAG_COMBINATION;
  }
}

ReturnValue_t VirtualChannelReception::frameAcceptanceAndReportingMechanism(TcTransferFrame* frame,
                                                                            ClcwIF* clcw) {
  ReturnValue_t result = returnvalue::OK;
  result = doFARM(frame, &internalClcw);
  internalClcw.setReceiverFrameSequenceNumber(vR);
  internalClcw.setFarmBCount(farmBCounter);
  clcw->setWhole(internalClcw.getAsWhole());
  switch (result) {
    case returnvalue::OK:
      return mapDemultiplexing(frame);
    case BC_IS_SET_VR_COMMAND:
    case BC_IS_UNLOCK_COMMAND:
      // Need to catch these codes to avoid error reporting later.
      return returnvalue::OK;
    default:
      break;
  }
  return result;
}

ReturnValue_t VirtualChannelReception::addMapChannel(uint8_t mapId, MapPacketExtractionIF* object) {
  std::pair<mapChannelIterator, bool> returnValue =
      mapChannels.insert(std::pair<uint8_t, MapPacketExtractionIF*>(mapId, object));
  if (returnValue.second == true) {
    return returnvalue::OK;
  } else {
    return returnvalue::FAILED;
  }
}

ReturnValue_t VirtualChannelReception::handleBDFrame(TcTransferFrame* frame, ClcwIF* clcw) {
  farmBCounter++;
  return returnvalue::OK;
}

ReturnValue_t VirtualChannelReception::handleBCFrame(TcTransferFrame* frame, ClcwIF* clcw) {
  BcFrame content;
  ReturnValue_t returnValue =
      content.initialize(frame->getFullDataField(), frame->getFullDataLength());
  if (returnValue == BC_IS_UNLOCK_COMMAND) {
    returnValue = currentState->handleBCUnlockCommand(clcw);
  } else if (returnValue == BC_IS_SET_VR_COMMAND) {
    returnValue = currentState->handleBCSetVrCommand(clcw, content.vR);
  } else {
    // Do nothing
  }
  return returnValue;
}

uint8_t VirtualChannelReception::getChannelId() const { return channelId; }

ReturnValue_t VirtualChannelReception::initialize() {
  ReturnValue_t returnValue = returnvalue::FAILED;
  if ((slidingWindowWidth > 254) || (slidingWindowWidth % 2 != 0)) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "VirtualChannelReception::initialize: Illegal sliding window width: "
               << (int)slidingWindowWidth << std::endl;
#endif
    return returnvalue::FAILED;
  }
  for (mapChannelIterator iterator = mapChannels.begin(); iterator != mapChannels.end();
       iterator++) {
    returnValue = iterator->second->initialize();
    if (returnValue != returnvalue::OK) break;
  }
  return returnValue;
}

void VirtualChannelReception::setToWaitState() {
  internalClcw.setWaitFlag(true);
  this->currentState = &waitState;
}
