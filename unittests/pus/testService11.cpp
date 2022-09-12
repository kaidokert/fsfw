#include <fsfw/pus/Service11TelecommandScheduling.h>

#include <catch2/catch_test_macros.hpp>

#include "objects/systemObjectList.h"
#include "tmtc/apid.h"
#include "tmtc/pusIds.h"

TEST_CASE("PUS Service 11", "[pus-srvc11]") {
  Service11TelecommandScheduling<13> pusService11(
      {objects::PUS_SERVICE_11_TC_SCHEDULER, apid::DEFAULT_APID, pus::PUS_SERVICE_11}, nullptr);

  // TODO test something...
}