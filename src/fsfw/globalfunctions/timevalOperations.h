#ifndef TIMEVALOPERATIONS_H_
#define TIMEVALOPERATIONS_H_

#include <fsfw/platform.h>
#include <stdint.h>

#ifdef PLATFORM_WIN
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

timeval& operator+=(timeval& lhs, const timeval& rhs);

timeval operator+(timeval lhs, const timeval& rhs);

timeval& operator-=(timeval& lhs, const timeval& rhs);

timeval operator-(timeval lhs, const timeval& rhs);

double operator/(const timeval& lhs, const timeval& rhs);

timeval& operator/=(timeval& lhs, double scalar);

timeval operator/(timeval lhs, double scalar);

timeval& operator*=(timeval& lhs, double scalar);

timeval operator*(timeval lhs, double scalar);

timeval operator*(double scalar, timeval rhs);

bool operator==(const timeval& lhs, const timeval& rhs);
bool operator!=(const timeval& lhs, const timeval& rhs);
bool operator<(const timeval& lhs, const timeval& rhs);
bool operator>(const timeval& lhs, const timeval& rhs);
bool operator<=(const timeval& lhs, const timeval& rhs);
bool operator>=(const timeval& lhs, const timeval& rhs);

namespace timevalOperations {
/**
 * returns the seconds and subseconds stored in the timeval
 * as double [s]
 *
 *
 * @param timeval
 * @return seconds
 */
double toDouble(const timeval timeval);
timeval toTimeval(const double seconds);
}  // namespace timevalOperations

#endif /* TIMEVALOPERATIONS_H_ */
