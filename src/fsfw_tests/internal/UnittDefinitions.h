#ifndef UNITTEST_INTERNAL_UNITTDEFINITIONS_H_
#define UNITTEST_INTERNAL_UNITTDEFINITIONS_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

namespace tv {
// POD test values
static const bool tv_bool = true;
static const uint8_t tv_uint8{5};
static const uint16_t tv_uint16{283};
static const uint32_t tv_uint32{929221};
static const uint64_t tv_uint64{2929329429};

static const int8_t tv_int8{-16};
static const int16_t tv_int16{-829};
static const int32_t tv_int32{-2312};

static const float tv_float{8.2149214};
static const float tv_sfloat = {-922.2321321};
static const double tv_double{9.2132142141e8};
static const double tv_sdouble{-2.2421e19};
}  // namespace tv

namespace unitt {
ReturnValue_t put_error(std::string errorId);
}

#endif /* UNITTEST_INTERNAL_UNITTDEFINITIONS_H_ */
