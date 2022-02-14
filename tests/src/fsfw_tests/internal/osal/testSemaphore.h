#ifndef UNITTEST_INTERNAL_INTTESTSEMAPHORE_H_
#define UNITTEST_INTERNAL_INTTESTSEMAPHORE_H_
class SemaphoreIF;
#include <string>

namespace testsemaph {
void testBinSemaph();
void testBinSemaphoreImplementation(SemaphoreIF* binSemaph, std::string id);
void testCountingSemaph();
void testCountingSemaphImplementation(SemaphoreIF* countingSemaph, std::string id);
}  // namespace testsemaph

#endif /* UNITTEST_INTERNAL_INTTESTSEMAPHORE_H_ */
