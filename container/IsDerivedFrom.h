#ifndef ISDERIVEDFROM_H_
#define ISDERIVEDFROM_H_

template<typename D, typename B>
class IsDerivedFrom {
	class No {
	};
	class Yes {
		No no[3];
	};

	static Yes Test(B*); // declared, but not defined
	static No Test(... ); // declared, but not defined

public:
	enum {
		Is = sizeof(Test(static_cast<D*>(0))) == sizeof(Yes)
	};
};

template<typename, typename>
struct is_same {
    static bool const value = false;
};

template<typename A>
struct is_same<A, A> {
    static bool const value = true;
};


template<bool C, typename T = void>
struct enable_if {
  typedef T type;
};

template<typename T>
struct enable_if<false, T> { };


#endif /* ISDERIVEDFROM_H_ */
