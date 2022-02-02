#ifndef SIGN_H_
#define SIGN_H_

template <typename T>
int sign(T val) {
  return (T(0) < val) - (val < T(0));
}

#endif /* SIGN_H_ */
