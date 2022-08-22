#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/SharedLocalDataSet.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/globalfunctions/bitutility.h>
#include <fsfw/objectmanager/ObjectManager.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"
#include "mocks/LocalPoolOwnerBase.h"
#include "mocks/MessageQueueMock.h"
#include "tests/TestsConfig.h"

using namespace returnvalue;

TEST_CASE("DataSetTest", "[DataSetTest]") {
  auto queue = MessageQueueMock(1);
  LocalPoolOwnerBase poolOwner(queue, objects::TEST_LOCAL_POOL_OWNER_BASE);
  REQUIRE(poolOwner.initializeHkManager() == OK);
  REQUIRE(poolOwner.initializeHkManagerAfterTaskCreation() == OK);
  LocalPoolStaticTestDataSet localSet;

  SECTION("BasicTest") {
    /* Test some basic functions */
    CHECK(localSet.getReportingEnabled() == false);
    CHECK(localSet.getLocalPoolIdsSerializedSize(false) == 3 * sizeof(lp_id_t));
    CHECK(localSet.getLocalPoolIdsSerializedSize(true) == 3 * sizeof(lp_id_t) + sizeof(uint8_t));
    CHECK(localSet.getSid() == lpool::testSid);
    CHECK(localSet.getCreatorObjectId() == objects::TEST_LOCAL_POOL_OWNER_BASE);
    size_t maxSize = localSet.getLocalPoolIdsSerializedSize(true);
    uint8_t localPoolIdBuff[maxSize];
    /* Skip size field */
    auto* lpIds = reinterpret_cast<lp_id_t*>(localPoolIdBuff + 1);
    size_t serSize = 0;
    auto* localPoolIdBuffPtr = reinterpret_cast<uint8_t*>(localPoolIdBuff);

    /* Test local pool ID serialization */
    CHECK(localSet.serializeLocalPoolIds(&localPoolIdBuffPtr, &serSize, maxSize,
                                         SerializeIF::Endianness::MACHINE) == returnvalue::OK);
    CHECK(serSize == maxSize);
    CHECK(localPoolIdBuff[0] == 3);
    CHECK(lpIds[0] == localSet.localPoolVarUint8.getDataPoolId());
    CHECK(lpIds[1] == localSet.localPoolVarFloat.getDataPoolId());
    CHECK(lpIds[2] == localSet.localPoolUint16Vec.getDataPoolId());
    /* Now serialize without fill count */
    lpIds = reinterpret_cast<lp_id_t*>(localPoolIdBuff);
    localPoolIdBuffPtr = localPoolIdBuff;
    serSize = 0;
    CHECK(localSet.serializeLocalPoolIds(&localPoolIdBuffPtr, &serSize, maxSize,
                                         SerializeIF::Endianness::MACHINE, false) == OK);
    CHECK(serSize == maxSize - sizeof(uint8_t));
    CHECK(lpIds[0] == localSet.localPoolVarUint8.getDataPoolId());
    CHECK(lpIds[1] == localSet.localPoolVarFloat.getDataPoolId());
    CHECK(lpIds[2] == localSet.localPoolUint16Vec.getDataPoolId());

    {
      /* Test read operation. Values should be all zeros */
      PoolReadGuard readHelper(&localSet);
      REQUIRE(readHelper.getReadResult() == returnvalue::OK);
      CHECK(not localSet.isValid());
      CHECK(localSet.localPoolVarUint8.value == 0);
      CHECK(not localSet.localPoolVarUint8.isValid());
      CHECK(localSet.localPoolVarFloat.value == Catch::Approx(0.0));
      CHECK(not localSet.localPoolVarUint8.isValid());
      CHECK(localSet.localPoolUint16Vec.value[0] == 0);
      CHECK(localSet.localPoolUint16Vec.value[1] == 0);
      CHECK(localSet.localPoolUint16Vec.value[2] == 0);
      CHECK(not localSet.localPoolVarUint8.isValid());

      /* Now set new values, commit should be done by read helper automatically */
      localSet.localPoolVarUint8 = 232;
      localSet.localPoolVarFloat = -2324.322;
      localSet.localPoolUint16Vec.value[0] = 232;
      localSet.localPoolUint16Vec.value[1] = 23923;
      localSet.localPoolUint16Vec.value[2] = 1;
      localSet.setValidity(true, true);
    }

    /* Zero out some values for next test */
    localSet.localPoolVarUint8 = 0;
    localSet.localPoolVarFloat = 0;

    localSet.setAllVariablesReadOnly();
    CHECK(localSet.localPoolUint16Vec.getReadWriteMode() == pool_rwm_t::VAR_READ);
    CHECK(localSet.localPoolVarUint8.getReadWriteMode() == pool_rwm_t::VAR_READ);
    CHECK(localSet.localPoolVarFloat.getReadWriteMode() == pool_rwm_t::VAR_READ);

    {
      /* Now we read again and check whether our zeroed values were overwritten with
      the values in the pool */
      PoolReadGuard readHelper(&localSet);
      REQUIRE(readHelper.getReadResult() == returnvalue::OK);
      CHECK(localSet.isValid());
      CHECK(localSet.localPoolVarUint8.value == 232);
      CHECK(localSet.localPoolVarUint8.isValid());
      CHECK(localSet.localPoolVarFloat.value == Catch::Approx(-2324.322));
      CHECK(localSet.localPoolVarFloat.isValid());
      CHECK(localSet.localPoolUint16Vec.value[0] == 232);
      CHECK(localSet.localPoolUint16Vec.value[1] == 23923);
      CHECK(localSet.localPoolUint16Vec.value[2] == 1);
      CHECK(localSet.localPoolUint16Vec.isValid());

      /* Now we serialize these values into a buffer without the validity buffer */
      localSet.setValidityBufferGeneration(false);
      maxSize = localSet.getSerializedSize();
      CHECK(maxSize == sizeof(uint8_t) + sizeof(uint16_t) * 3 + sizeof(float));
      serSize = 0;
      /* Already reserve additional space for validity buffer, will be needed later */
      uint8_t buffer[maxSize + 1];
      uint8_t* buffPtr = buffer;
      CHECK(localSet.serialize(&buffPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) ==
            returnvalue::OK);
      uint8_t rawUint8 = buffer[0];
      CHECK(rawUint8 == 232);
      float rawFloat = 0.0;
      std::memcpy(&rawFloat, buffer + sizeof(uint8_t), sizeof(float));
      CHECK(rawFloat == Catch::Approx(-2324.322));

      uint16_t rawUint16Vec[3];
      std::memcpy(&rawUint16Vec, buffer + sizeof(uint8_t) + sizeof(float), 3 * sizeof(uint16_t));
      CHECK(rawUint16Vec[0] == 232);
      CHECK(rawUint16Vec[1] == 23923);
      CHECK(rawUint16Vec[2] == 1);

      size_t sizeToDeserialize = maxSize;
      /* Now we zeros out the raw entries and deserialize back into the dataset */
      std::memset(buffer, 0, sizeof(buffer));
      const uint8_t* constBuffPtr = buffer;
      CHECK(localSet.deSerialize(&constBuffPtr, &sizeToDeserialize,
                                 SerializeIF::Endianness::MACHINE) == returnvalue::OK);
      /* Check whether deserialization was successfull */
      CHECK(localSet.localPoolVarUint8.value == 0);
      CHECK(localSet.localPoolVarFloat.value == Catch::Approx(0.0));
      CHECK(localSet.localPoolVarUint8.value == 0);
      CHECK(localSet.localPoolUint16Vec.value[0] == 0);
      CHECK(localSet.localPoolUint16Vec.value[1] == 0);
      CHECK(localSet.localPoolUint16Vec.value[2] == 0);
      /* Validity should be unchanged */
      CHECK(localSet.localPoolVarUint8.isValid());
      CHECK(localSet.localPoolVarFloat.isValid());
      CHECK(localSet.localPoolUint16Vec.isValid());

      /* Now we do the same process but with the validity buffer */
      localSet.localPoolVarUint8 = 232;
      localSet.localPoolVarFloat = -2324.322;
      localSet.localPoolUint16Vec.value[0] = 232;
      localSet.localPoolUint16Vec.value[1] = 23923;
      localSet.localPoolUint16Vec.value[2] = 1;
      localSet.localPoolVarUint8.setValid(true);
      localSet.localPoolVarFloat.setValid(false);
      localSet.localPoolUint16Vec.setValid(true);
      localSet.setValidityBufferGeneration(true);
      maxSize = localSet.getSerializedSize();
      CHECK(maxSize == sizeof(uint8_t) + sizeof(uint16_t) * 3 + sizeof(float) + 1);
      serSize = 0;
      buffPtr = buffer;
      CHECK(localSet.serialize(&buffPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) ==
            returnvalue::OK);
      CHECK(rawUint8 == 232);
      std::memcpy(&rawFloat, buffer + sizeof(uint8_t), sizeof(float));
      CHECK(rawFloat == Catch::Approx(-2324.322));

      std::memcpy(&rawUint16Vec, buffer + sizeof(uint8_t) + sizeof(float), 3 * sizeof(uint16_t));
      CHECK(rawUint16Vec[0] == 232);
      CHECK(rawUint16Vec[1] == 23923);
      CHECK(rawUint16Vec[2] == 1);
      /* We can do it like this because the buffer only has one byte for
      less than 8 variables */
      uint8_t* validityByte = buffer + sizeof(buffer) - 1;
      bool bitSet = false;
      bitutil::get(validityByte, 0, bitSet);

      CHECK(bitSet == true);
      bitutil::get(validityByte, 1, bitSet);
      CHECK(bitSet == false);
      bitutil::get(validityByte, 2, bitSet);
      CHECK(bitSet == true);

      /* Now we manipulate the validity buffer for the deserialization */
      bitutil::clear(validityByte, 0);
      bitutil::set(validityByte, 1);
      bitutil::clear(validityByte, 2);
      /* Zero out everything except validity buffer */
      std::memset(buffer, 0, sizeof(buffer) - 1);
      sizeToDeserialize = maxSize;
      constBuffPtr = buffer;
      CHECK(localSet.deSerialize(&constBuffPtr, &sizeToDeserialize,
                                 SerializeIF::Endianness::MACHINE) == returnvalue::OK);
      /* Check whether deserialization was successfull */
      CHECK(localSet.localPoolVarUint8.value == 0);
      CHECK(localSet.localPoolVarFloat.value == Catch::Approx(0.0));
      CHECK(localSet.localPoolVarUint8.value == 0);
      CHECK(localSet.localPoolUint16Vec.value[0] == 0);
      CHECK(localSet.localPoolUint16Vec.value[1] == 0);
      CHECK(localSet.localPoolUint16Vec.value[2] == 0);
      CHECK(not localSet.localPoolVarUint8.isValid());
      CHECK(localSet.localPoolVarFloat.isValid());
      CHECK(not localSet.localPoolUint16Vec.isValid());
    }

    /* Common fault test cases */
    LocalPoolObjectBase* variableHandle = poolOwner.getPoolObjectHandle(lpool::uint32VarId);
    CHECK(variableHandle != nullptr);
    CHECK(localSet.registerVariable(variableHandle) == static_cast<int>(DataSetIF::DATA_SET_FULL));
    variableHandle = nullptr;
    REQUIRE(localSet.registerVariable(variableHandle) ==
            static_cast<int>(DataSetIF::POOL_VAR_NULL));
  }

  SECTION("MorePoolVariables") {
    LocalDataSet set(&poolOwner, 2, 10);

    /* Register same variables again to get more than 8 registered variables */
    for (uint8_t idx = 0; idx < 8; idx++) {
      REQUIRE(set.registerVariable(&localSet.localPoolVarUint8) == returnvalue::OK);
    }
    REQUIRE(set.registerVariable(&localSet.localPoolVarUint8) == returnvalue::OK);
    REQUIRE(set.registerVariable(&localSet.localPoolUint16Vec) == returnvalue::OK);

    set.setValidityBufferGeneration(true);
    {
      PoolReadGuard readHelper(&localSet);
      localSet.localPoolVarUint8.value = 42;
      localSet.localPoolVarUint8.setValid(true);
      localSet.localPoolUint16Vec.setValid(false);
    }

    size_t maxSize = set.getSerializedSize();
    CHECK(maxSize == 9 + sizeof(uint16_t) * 3 + 2);
    size_t serSize = 0;
    /* Already reserve additional space for validity buffer, will be needed later */
    uint8_t buffer[maxSize + 1];
    uint8_t* buffPtr = buffer;
    CHECK(set.serialize(&buffPtr, &serSize, maxSize, SerializeIF::Endianness::MACHINE) == OK);
    std::array<uint8_t, 2> validityBuffer{};
    std::memcpy(validityBuffer.data(), buffer + 9 + sizeof(uint16_t) * 3, 2);
    /* The first 9 variables should be valid */
    CHECK(validityBuffer[0] == 0xff);
    bool bitSet = false;
    bitutil::get(validityBuffer.data() + 1, 0, bitSet);
    CHECK(bitSet == true);
    bitutil::get(validityBuffer.data() + 1, 1, bitSet);
    CHECK(bitSet == false);

    /* Now we invert the validity */
    validityBuffer[0] = 0;
    validityBuffer[1] = 0b0100'0000;
    std::memcpy(buffer + 9 + sizeof(uint16_t) * 3, validityBuffer.data(), 2);
    const uint8_t* constBuffPtr = buffer;
    size_t sizeToDeSerialize = serSize;
    CHECK(set.deSerialize(&constBuffPtr, &sizeToDeSerialize, SerializeIF::Endianness::MACHINE) ==
          returnvalue::OK);
    CHECK(localSet.localPoolVarUint8.isValid() == false);
    CHECK(localSet.localPoolUint16Vec.isValid() == true);
  }

  SECTION("SharedDataSet") {
    object_id_t sharedSetId = objects::SHARED_SET_ID;
    SharedLocalDataSet sharedSet(sharedSetId, &poolOwner, lpool::testSetId, 5);
    localSet.localPoolVarUint8.setReadWriteMode(pool_rwm_t::VAR_WRITE);
    localSet.localPoolUint16Vec.setReadWriteMode(pool_rwm_t::VAR_WRITE);
    CHECK(sharedSet.registerVariable(&localSet.localPoolVarUint8) == returnvalue::OK);
    CHECK(sharedSet.registerVariable(&localSet.localPoolUint16Vec) == returnvalue::OK);
    CHECK(sharedSet.initialize() == returnvalue::OK);
    CHECK(sharedSet.lockDataset() == returnvalue::OK);
    CHECK(sharedSet.unlockDataset() == returnvalue::OK);

    {
      // PoolReadGuard rg(&sharedSet);
      // CHECK(rg.getReadResult() == result::OK);
      localSet.localPoolVarUint8.value = 5;
      localSet.localPoolUint16Vec.value[0] = 1;
      localSet.localPoolUint16Vec.value[1] = 2;
      localSet.localPoolUint16Vec.value[2] = 3;
      CHECK(sharedSet.commit() == returnvalue::OK);
    }

    sharedSet.setReadCommitProtectionBehaviour(true);
  }
}
