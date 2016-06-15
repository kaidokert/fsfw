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
double toDouble(const timeval timeval);
}

#endif /* TIMEVALOPERATIONS_H_ */
