#ifndef FSFW_MODES_HASMODESIF_H_
#define FSFW_MODES_HASMODESIF_H_

#include <cstdint>

#include "../events/Event.h"
#include "../returnvalues/returnvalue.h"
#include "ModeHelper.h"
#include "ModeMessage.h"

class HasModesIF {
  friend class ModeHelper;

 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::HAS_MODES_IF;
  static const ReturnValue_t INVALID_MODE = MAKE_RETURN_CODE(0x01);
  static const ReturnValue_t TRANS_NOT_ALLOWED = MAKE_RETURN_CODE(0x02);
  static const ReturnValue_t IN_TRANSITION = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t INVALID_SUBMODE = MAKE_RETURN_CODE(0x04);

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SYSTEM_MANAGER;
  static const Event CHANGING_MODE =
      MAKE_EVENT(0, severity::INFO);  //!< An object announces changing the mode. p1: target mode.
                                      //!< p2: target submode
  static const Event MODE_INFO = MAKE_EVENT(
      1,
      severity::INFO);  //!< An Object announces its mode; parameter1 is mode, parameter2 is submode
  static const Event FALLBACK_FAILED = MAKE_EVENT(2, severity::HIGH);
  static const Event MODE_TRANSITION_FAILED = MAKE_EVENT(3, severity::LOW);
  static const Event CANT_KEEP_MODE = MAKE_EVENT(4, severity::HIGH);
  static const Event OBJECT_IN_INVALID_MODE =
      MAKE_EVENT(5, severity::LOW);  //!< Indicates a bug or configuration failure: Object is in a
                                     //!< mode it should never be in.
  static const Event FORCING_MODE = MAKE_EVENT(
      6, severity::MEDIUM);  //!< The mode is changed, but for some reason, the change is forced,
                             //!< i.e. EXTERNAL_CONTROL ignored. p1: target mode. p2: target submode
  static const Event MODE_CMD_REJECTED =
      MAKE_EVENT(7, severity::LOW);  //!< A mode command was rejected by the called object. Par1:
                                     //!< called object id, Par2: return code.

  static const Mode_t MODE_ON =
      1;  //!< The device is powered and ready to perform operations. In this mode, no commands are
          //!< sent by the device handler itself, but direct commands van be commanded and will be
          //!< interpreted
  static const Mode_t MODE_OFF = 0;  //!< The device is powered off. The only command accepted in
                                     //!< this mode is a mode change to on.
  static const Submode_t SUBMODE_NONE = 0;  //!< To avoid checks against magic number "0".

  virtual ~HasModesIF() {}
  virtual MessageQueueId_t getCommandQueue() const = 0;
  virtual void getMode(Mode_t *mode, Submode_t *submode) = 0;

 protected:
  virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                         uint32_t *msToReachTheMode) {
    return returnvalue::FAILED;
  }

  virtual void startTransition(Mode_t mode, Submode_t submode) {}

  virtual void setToExternalControl() {}

  virtual void announceMode(bool recursive) {}
};

#endif /*FSFW_MODES_HASMODESIF_H_ */
