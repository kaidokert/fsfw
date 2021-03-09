#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/globalfunctions/bitutility.h>

#include <unittest/core/CatchDefinitions.h>

TEST_CASE("LocalDataSet" , "[LocDataSetTest]") {
    LocalPoolOwnerBase* poolOwner = objectManager->
            get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
    REQUIRE(poolOwner != nullptr);
    REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
    REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
            == retval::CATCH_OK);
    LocalPoolStaticTestDataSet localSet;

    SECTION("BasicTest") {
        /* Test some basic functions */
        CHECK(localSet.getReportingEnabled() == false);
        CHECK(localSet.getLocalPoolIdsSerializedSize(false) == 3 * sizeof(lp_id_t));
        CHECK(localSet.getLocalPoolIdsSerializedSize(true) ==
                3 * sizeof(lp_id_t) + sizeof(uint8_t));
        CHECK(localSet.getSid() == lpool::testSid);
        CHECK(localSet.getCreatorObjectId() == objects::TEST_LOCAL_POOL_OWNER_BASE);
        size_t maxSize = localSet.getLocalPoolIdsSerializedSize(true);
        uint8_t localPoolIdBuff[maxSize];
        /* Skip size field */
        lp_id_t* lpIds = reinterpret_cast<lp_id_t*>(localPoolIdBuff + 1);
        size_t serSize = 0;
        uint8_t *localPoolIdBuffPtr = reinterpret_cast<uint8_t*>(localPoolIdBuff);

        /* Test local pool ID serialization */
        CHECK(localSet.serializeLocalPoolIds(&localPoolIdBuffPtr, &serSize,
                        maxSize, SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
        CHECK(serSize == maxSize);
        CHECK(localPoolIdBuff[0] == 3);
        CHECK(lpIds[0] == localSet.localPoolVarUint8.getDataPoolId());
        CHECK(lpIds[1]  == localSet.localPoolVarFloat.getDataPoolId());
        CHECK(lpIds[2]  == localSet.localPoolUint16Vec.getDataPoolId());
        /* Now serialize without fill count */
        lpIds = reinterpret_cast<lp_id_t*>(localPoolIdBuff);
        localPoolIdBuffPtr = localPoolIdBuff;
        serSize = 0;
        CHECK(localSet.serializeLocalPoolIds(&localPoolIdBuffPtr, &serSize,
                        maxSize, SerializeIF::Endianness::MACHINE, false) == retval::CATCH_OK);
        CHECK(serSize == maxSize - sizeof(uint8_t));
        CHECK(lpIds[0] == localSet.localPoolVarUint8.getDataPoolId());
        CHECK(lpIds[1]  == localSet.localPoolVarFloat.getDataPoolId());
        CHECK(lpIds[2]  == localSet.localPoolUint16Vec.getDataPoolId());

        {
            /* Test read operation. Values should be all zeros */
            PoolReadGuard readHelper(&localSet);
            REQUIRE(readHelper.getReadResult() == retval::CATCH_OK);
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

        {
            /* Now we read again and check whether our zeroed values were overwritten with
            the values in the pool */
            PoolReadGuard readHelper(&localSet);
            REQUIRE(readHelper.getReadResult() == retval::CATCH_OK);
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
            CHECK(localSet.serialize(&buffPtr, &serSize, maxSize,
                    SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
            uint8_t rawUint8 = buffer[0];
            CHECK(rawUint8 == 232);
            float rawFloat = 0.0;
            std::memcpy(&rawFloat, buffer + sizeof(uint8_t), sizeof(float));
            CHECK(rawFloat == Catch::Approx(-2324.322));

            uint16_t rawUint16Vec[3];
            std::memcpy(&rawUint16Vec, buffer + sizeof(uint8_t) + sizeof(float),
                    3 * sizeof(uint16_t));
            CHECK(rawUint16Vec[0] == 232);
            CHECK(rawUint16Vec[1] == 23923);
            CHECK(rawUint16Vec[2] == 1);

            size_t sizeToDeserialize = maxSize;
            /* Now we zeros out the raw entries and deserialize back into the dataset */
            std::memset(buffer, 0, sizeof(buffer));
            const uint8_t* constBuffPtr = buffer;
            CHECK(localSet.deSerialize(&constBuffPtr, &sizeToDeserialize,
                    SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
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
            CHECK(localSet.serialize(&buffPtr, &serSize, maxSize,
                    SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
            CHECK(rawUint8 == 232);
            std::memcpy(&rawFloat, buffer + sizeof(uint8_t), sizeof(float));
            CHECK(rawFloat == Catch::Approx(-2324.322));

            std::memcpy(&rawUint16Vec, buffer + sizeof(uint8_t) + sizeof(float),
                    3 * sizeof(uint16_t));
            CHECK(rawUint16Vec[0] == 232);
            CHECK(rawUint16Vec[1] == 23923);
            CHECK(rawUint16Vec[2] == 1);
            /* We can do it like this because the buffer only has one byte for
            less than 8 variables */
            uint8_t* validityByte = buffer + sizeof(buffer) - 1;
            CHECK(bitutil::bitGet(validityByte, 0) == true);
            CHECK(bitutil::bitGet(validityByte, 1) == false);
            CHECK(bitutil::bitGet(validityByte, 2) == true);

            /* Now we manipulate the validity buffer for the deserialization */
            bitutil::bitClear(validityByte, 0);
            bitutil::bitSet(validityByte, 1);
            bitutil::bitClear(validityByte, 2);
            /* Zero out everything except validity buffer */
            std::memset(buffer, 0, sizeof(buffer) - 1);
            sizeToDeserialize = maxSize;
            constBuffPtr = buffer;
            CHECK(localSet.deSerialize(&constBuffPtr, &sizeToDeserialize,
                    SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
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
        LocalPoolObjectBase* variableHandle = poolOwner->getPoolObjectHandle(lpool::uint32VarId);
        CHECK(variableHandle != nullptr);
        CHECK(localSet.registerVariable(variableHandle) ==
                static_cast<int>(DataSetIF::DATA_SET_FULL));
        variableHandle = nullptr;
        REQUIRE(localSet.registerVariable(variableHandle) ==
                static_cast<int>(DataSetIF::POOL_VAR_NULL));

    }

    /* we need to reset the subscription list because the pool owner
    is a global object. */
    CHECK(poolOwner->reset() == retval::CATCH_OK);
}



