#include "fsfw/health/HealthMessage.h"

void HealthMessage::setHealthMessage(CommandMessage* message, Command_t command,
                                     HasHealthIF::HealthState health,
                                     HasHealthIF::HealthState oldHealth) {
  message->setCommand(command);
  message->setParameter(health);
  message->setParameter2(oldHealth);
}

void HealthMessage::setHealthMessage(CommandMessage* message, Command_t command) {
  message->setCommand(command);
}

HasHealthIF::HealthState HealthMessage::getHealth(const CommandMessage* message) {
  return (HasHealthIF::HealthState)message->getParameter();
}

void HealthMessage::clear(CommandMessage* message) {
  message->setCommand(CommandMessage::CMD_NONE);
}

HealthMessage::HealthMessage() {}

HasHealthIF::HealthState HealthMessage::getOldHealth(const CommandMessage* message) {
  return (HasHealthIF::HealthState)message->getParameter2();
}
