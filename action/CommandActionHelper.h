
#ifndef COMMANDACTIONHELPER_H_
#define COMMANDACTIONHELPER_H_

#include <framework/action/ActionMessage.h>
#include <framework/ipc/MessageQueue.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/SerializeIF.h>
#include <framework/storagemanager/StorageManagerIF.h>

class CommandsActionsIF;

class CommandActionHelper {
	friend class CommandsActionsIF;
public:
	CommandActionHelper(CommandsActionsIF* owner);
	virtual ~CommandActionHelper();
	ReturnValue_t commandAction(object_id_t commandTo,
			ActionId_t actionId, const uint8_t* data, uint32_t size);
	ReturnValue_t commandAction(object_id_t commandTo,
			ActionId_t actionId, SerializeIF* data);
	ReturnValue_t initialize();
	ReturnValue_t handleReply(CommandMessage* reply);
	uint8_t getCommandCount() const;
private:
	CommandsActionsIF* owner;
	MessageQueue* queueToUse;
	StorageManagerIF* ipcStore;
	uint8_t commandCount;
	MessageQueueId_t lastTarget;
	void extractDataForOwner(ActionId_t actionId, store_address_t storeId);
	ReturnValue_t sendCommand(MessageQueueId_t queueId, ActionId_t actionId,
			store_address_t storeId);
};

#endif /* COMMANDACTIONHELPER_H_ */
