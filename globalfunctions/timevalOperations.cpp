#include <framework/globalfunctions/timevalOperations.h>
#include <sys/time.h>
#include <stdint.h>

//TODO test with large numbers

timeval& operator+=(timeval& lhs, const timeval& rhs) {
	int64_t sum = lhs.tv_sec * 1000000. + lhs.tv_usec;
	sum += rhs.tv_sec * 1000000. + rhs.tv_usec;
	lhs.tv_sec = sum / 1000000;
	lhs.tv_usec = sum - lhs.tv_sec * 1000000;
	return lhs;
}

timeval operator+(timeval lhs, const timeval& rhs) {
	lhs += rhs;
	return lhs;
}

timeval& operator-=(timeval& lhs, const timeval& rhs) {
	int64_t sum = lhs.tv_sec * 1000000. + lhs.tv_usec;
	sum -= rhs.tv_sec * 1000000. + rhs.tv_usec;
	lhs.tv_sec = sum / 1000000;
	lhs.tv_usec = sum - lhs.tv_sec * 1000000;
	return lhs;
}

timeval operator-(timeval lhs, const timeval& rhs) {
	lhs -= rhs;
	return lhs;
}

double operator/(const timeval& lhs, const timeval& rhs) {
	double lhs64 = lhs.tv_sec * 1000000. + lhs.tv_usec;
	double rhs64 = rhs.tv_sec * 1000000. + rhs.tv_usec;
	return lhs64 / rhs64;
}

timeval& operator/=(timeval& lhs, double scalar) {
	int64_t product = lhs.tv_sec * 1000000. + lhs.tv_usec;
	product /= scalar;
	lhs.tv_sec = product / 1000000;
	lhs.tv_usec = product - lhs.tv_sec * 1000000;
	return lhs;
}

timeval operator/(timeval lhs, double scalar) {
	lhs /= scalar;
	return lhs;
}

timeval& operator*=(timeval& lhs, double scalar) {
	int64_t product = lhs.tv_sec * 1000000. + lhs.tv_usec;
	product *= scalar;
	lhs.tv_sec = product / 1000000;
	lhs.tv_usec = product - lhs.tv_sec * 1000000;
	return lhs;
}

timeval operator*(timeval lhs, double scalar) {
	lhs *= scalar;
	return lhs;
}

timeval operator*(double scalar, timeval rhs) {
	rhs *= scalar;
	return rhs;
}

bool operator==(const timeval& lhs, const timeval& rhs) {
	int64_t lhs64 = lhs.tv_sec * 1000000. + lhs.tv_usec;
	int64_t rhs64 = rhs.tv_sec * 1000000. + rhs.tv_usec;
	return lhs64 == rhs64;
}
bool operator!=(const timeval& lhs, const timeval& rhs) {
	return !operator==(lhs, rhs);
}
bool operator<(const timeval& lhs, const timeval& rhs) {
	int64_t lhs64 = lhs.tv_sec * 1000000. + lhs.tv_usec;
	int64_t rhs64 = rhs.tv_sec * 1000000. + rhs.tv_usec;
	return lhs64 < rhs64;
}
bool operator>(const timeval& lhs, const timeval& rhs) {
	return operator<(rhs, lhs);
}
bool operator<=(const timeval& lhs, const timeval& rhs) {
	return !operator>(lhs, rhs);
}
bool operator>=(const timeval& lhs, const timeval& rhs) {
	return !operator<(lhs, rhs);
}

double timevalOperations::toDouble(const timeval timeval) {
	double result = timeval.tv_sec * 1000000. + timeval.tv_usec;
	return result / 1000000.;
}
