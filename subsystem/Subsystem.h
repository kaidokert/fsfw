#ifndef SUBSYSTEM_H_
#define SUBSYSTEM_H_

#include "../container/FixedArrayList.h"
#include "../container/FixedMap.h"
#include "../container/HybridIterator.h"
#include "../container/SinglyLinkedList.h"
#include "../serialize/SerialArrayListAdapter.h"
#include "modes/ModeDefinitions.h"
#include "SubsystemBase.h"

class Subsystem: public SubsystemBase, public HasModeSequenceIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SUBSYSTEM;
	static const ReturnValue_t SEQUENCE_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t TABLE_ALREADY_EXISTS = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t TABLE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t TABLE_OR_SEQUENCE_LENGTH_INVALID = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t SEQUENCE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x05);
	static const ReturnValue_t TABLE_CONTAINS_INVALID_OBJECT_ID =
			MAKE_RETURN_CODE(0x06);
	static const ReturnValue_t FALLBACK_SEQUENCE_DOES_NOT_EXIST =
			MAKE_RETURN_CODE(0x07);
	static const ReturnValue_t NO_TARGET_TABLE = MAKE_RETURN_CODE(0x08);
	static const ReturnValue_t SEQUENCE_OR_TABLE_TOO_LONG = MAKE_RETURN_CODE(0x09);
	static const ReturnValue_t IS_FALLBACK_SEQUENCE = MAKE_RETURN_CODE(0x0B);
	static const ReturnValue_t ACCESS_DENIED = MAKE_RETURN_CODE(0x0C);
	static const ReturnValue_t TABLE_IN_USE = MAKE_RETURN_CODE(0x0E);

	static const ReturnValue_t TARGET_TABLE_NOT_REACHED = MAKE_RETURN_CODE(0xA1);
	static const ReturnValue_t TABLE_CHECK_FAILED = MAKE_RETURN_CODE(0xA2);



	Subsystem(object_id_t setObjectId, object_id_t parent,
			uint32_t maxNumberOfSequences, uint32_t maxNumberOfTables);
	virtual ~Subsystem();

	ReturnValue_t addSequence(ArrayList<ModeListEntry>* sequence, Mode_t id,
			Mode_t fallbackSequence, bool inStore = true, bool preInit = true);

	ReturnValue_t addTable(ArrayList<ModeListEntry> *table, Mode_t id,
			bool inStore = true, bool preInit = true);

	void setInitialMode(Mode_t mode);

	virtual ReturnValue_t initialize();

	virtual ReturnValue_t checkObjectConnections();

	virtual MessageQueueId_t getSequenceCommandQueue() const;

	/**
	 *
	 *
	 * IMPORTANT: Do not call on non existing sequence! Use existsSequence() first
	 *
	 * @param sequence
	 * @return
	 */
	ReturnValue_t checkSequence(Mode_t sequence);

	/**
	 *
	 *
	 * IMPORTANT: Do not call on non existing sequence! Use existsSequence() first
	 *
	 * @param iter
	 * @return
	 */
	ReturnValue_t checkSequence(HybridIterator<ModeListEntry> iter, Mode_t fallbackSequence);
protected:

	struct EntryPointer {
		bool islinked;
		union {
			ModeListEntry *firstLinkedElement;
			ArrayList<ModeListEntry> *array;
		};
	};

	struct SequenceInfo {
		Mode_t fallbackSequence;
		EntryPointer entries;
	};

	static const uint8_t MAX_NUMBER_OF_TABLES_OR_SEQUENCES = 70;

	static const uint8_t MAX_LENGTH_OF_TABLE_OR_SEQUENCE = 20;

	bool isInTransition;

	bool childrenChangedHealth;

	uint32_t uptimeStartTable;

	HybridIterator<ModeListEntry> currentTargetTable;

	Mode_t targetMode;

	Submode_t targetSubmode;

	Mode_t initialMode;

	HybridIterator<ModeListEntry> currentSequenceIterator;

	FixedMap<Mode_t, EntryPointer> modeTables;

	FixedMap<Mode_t, SequenceInfo> modeSequences;

	StorageManagerIF *IPCStore;

#ifdef USE_MODESTORE
	ModeStoreIF *modeStore;
#endif

	bool existsModeSequence(Mode_t id);

	HybridIterator<ModeListEntry> getSequence(Mode_t id);

	bool existsModeTable(Mode_t id);

	HybridIterator<ModeListEntry> getTable(Mode_t id);

	HybridIterator<ModeListEntry> getCurrentTable();

//	void startSequence(Mode_t sequence);

	/**
	 * DO NOT USE ON NON EXISTING SEQUENCE
	 *
	 * @param a sequence
	 * @return the fallback sequence's Id
	 */
	Mode_t getFallbackSequence(Mode_t sequence);

	void replyToCommand(ReturnValue_t status, uint32_t parameter);

	ReturnValue_t deleteSequence(Mode_t id);

	ReturnValue_t deleteTable(Mode_t id);

	virtual void performChildOperation();

	virtual ReturnValue_t handleCommandMessage(CommandMessage *message);

	bool isFallbackSequence(Mode_t SequenceId);

	bool isTableUsed(Mode_t tableId);

	virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
			uint32_t *msToReachTheMode);

	virtual void startTransition(Mode_t mode, Submode_t submode);

	void sendSerializablesAsCommandMessage(Command_t command, SerializeIF **elements, uint8_t count);

	void transitionFailed(ReturnValue_t failureCode, uint32_t parameter);

	void cantKeepMode();

};

#endif /* SUBSYSTEM_H_ */
