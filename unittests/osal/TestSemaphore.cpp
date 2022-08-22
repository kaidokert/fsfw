
#ifdef LINUX

/*
#include <fsfw/tasks/SemaphoreFactory.h>
#include <fsfw/timemanager/Stopwatch.h>

#include "catch.hpp"
#include "core/CatchDefinitions.h"

TEST_CASE("Binary Semaphore Test" , "[BinSemaphore]") {
        //perform set-up here
        SemaphoreIF* binSemaph = SemaphoreFactory::instance()->
                        createBinarySemaphore();
        REQUIRE(binSemaph != nullptr);
        SECTION("Simple Test") {
                // set-up is run for each section
                REQUIRE(binSemaph->getSemaphoreCounter() == 1);
                REQUIRE(binSemaph->release() ==
                                static_cast<int>(SemaphoreIF::SEMAPHORE_NOT_OWNED));
                REQUIRE(binSemaph->acquire(SemaphoreIF::POLLING) ==
                                result::OK);
                {
                        // not precise enough on linux.. should use clock instead..
                        //Stopwatch stopwatch(false);
                        //REQUIRE(binSemaph->acquire(SemaphoreIF::TimeoutType::WAITING, 5) ==
                        //		SemaphoreIF::SEMAPHORE_TIMEOUT);
                        //dur_millis_t time = stopwatch.stop();
                        //CHECK(time == 5);
                }
                REQUIRE(binSemaph->getSemaphoreCounter() == 0);
                REQUIRE(binSemaph->release() == result::OK);
        }
        SemaphoreFactory::instance()->deleteSemaphore(binSemaph);
        // perform tear-down here
}


TEST_CASE("Counting Semaphore Test" , "[CountingSemaph]") {
        SECTION("Simple Test") {

        }
}
*/

#endif
