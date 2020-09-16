#ifndef TIMEVALOPERATIONS_H_
#define TIMEVALOPERATIONS_H_

#include <stdint.h>
#include <sys/time.h>

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
}

#endif /* TIMEVALOPERATIONS_H_ */
