#ifndef MISSION_ASSEMBLIES_TESTASSEMBLY_H_
#define MISSION_ASSEMBLIES_TESTASSEMBLY_H_

#include <fsfw/devicehandlers/AssemblyBase.h>

#include "../devices/TestDeviceHandler.h"

class TestAssembly : public AssemblyBase {
 public:
  TestAssembly(object_id_t objectId, object_id_t parentId, object_id_t testDevice0,
               object_id_t testDevice1);
  virtual ~TestAssembly();
  ReturnValue_t initialize() override;

  enum submodes : Submode_t { SINGLE = 0, DUAL = 1 };

 protected:
  /**
   * Command children to reach [mode,submode] combination
   * Can be done by setting #commandsOutstanding correctly,
   * or using executeTable()
   * @param mode
   * @param submode
   * @return
   *    - @c returnvalue::OK if ok
   *    - @c NEED_SECOND_STEP if children need to be commanded again
   */
  ReturnValue_t commandChildren(Mode_t mode, Submode_t submode) override;
  /**
   * Check whether desired assembly mode was achieved by checking the modes
   * or/and health states of child device handlers.
   * The assembly template class will also call this function if a health
   * or mode change of a child device handler was detected.
   * @param wantedMode
   * @param wantedSubmode
   * @return
   */
  ReturnValue_t isModeCombinationValid(Mode_t mode, Submode_t submode) override;

  ReturnValue_t checkChildrenStateOn(Mode_t wantedMode, Submode_t wantedSubmode) override;

 private:
  FixedArrayList<ModeListEntry, 2> commandTable;
  object_id_t deviceHandler0Id = 0;
  object_id_t deviceHandler1Id = 0;
  TestDevice* handler0 = nullptr;
  TestDevice* handler1 = nullptr;

  bool isDeviceAvailable(object_id_t object);
};

#endif /* MISSION_ASSEMBLIES_TESTASSEMBLY_H_ */
