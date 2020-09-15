#ifndef CCSDSTIME_H_
#define CCSDSTIME_H_

// COULDDO: have calls in Clock.h which return time quality and use timespec accordingly

#include "Clock.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include <cstdint>

bool operator<(const timeval& lhs, const timeval& rhs);
bool operator<=(const timeval& lhs, const timeval& rhs);
bool operator==(const timeval& lhs, const timeval& rhs);
/**
 * static helper class for CCSDS Time Code Formats
 *
 * as described in CCSDS 301.0-B-3
 *
 * Still work in progress
 */
class CCSDSTime: public HasReturnvaluesIF {
public:
	/**
	 * The Time code identifications, bits 4-6 in the P-Field
	 */
	enum TimeCodeIdentification {
		CCS = 0b101,
		CUC_LEVEL1 = 0b001,
		CUC_LEVEL2 = 0b010,
		CDS = 0b100,
		AGENCY_DEFINED = 0b110
	};
	static const uint8_t P_FIELD_CUC_6B_CCSDS = (CUC_LEVEL1 << 4) + (3 << 2)
			+ 2;
	static const uint8_t P_FIELD_CUC_6B_AGENCY = (CUC_LEVEL2 << 4) + (3 << 2)
			+ 2;
	static const uint8_t P_FIELD_CDS_SHORT = (CDS << 4);
	/**
	 * Struct for CDS day-segmented format.
	 */
	struct CDS_short {
		uint8_t pField;
		uint8_t dayMSB;
		uint8_t dayLSB;
		uint8_t msDay_hh;
		uint8_t msDay_h;
		uint8_t msDay_l;
		uint8_t msDay_ll;
	};
	/**
	 * Struct for the CCS fromat in day of month variation with max resolution
	 */
	struct Ccs_seconds {
		uint8_t pField;
		uint8_t yearMSB;
		uint8_t yearLSB;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	};

	/**
	 * Struct for the CCS fromat in day of month variation with 10E-4 seconds resolution
	 */
	struct Ccs_mseconds {
		uint8_t pField;
		uint8_t yearMSB;
		uint8_t yearLSB;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint8_t secondEminus2;
		uint8_t secondEminus4;
	};

	struct OBT_FLP {
		uint8_t pFiled;
		uint8_t seconds_hh;
		uint8_t seconds_h;
		uint8_t seconds_l;
		uint8_t seconds_ll;
		uint8_t subsecondsMSB;
		uint8_t subsecondsLSB;
	};

	struct TimevalLess {
		bool operator()(const timeval& lhs, const timeval& rhs) const {
			return (lhs < rhs);
		}
	};

	static const uint8_t INTERFACE_ID = CLASS_ID::CCSDS_TIME_HELPER_CLASS;
	static const ReturnValue_t UNSUPPORTED_TIME_FORMAT = MAKE_RETURN_CODE(0);
	static const ReturnValue_t NOT_ENOUGH_INFORMATION_FOR_TARGET_FORMAT =
			MAKE_RETURN_CODE(1);
	static const ReturnValue_t LENGTH_MISMATCH = MAKE_RETURN_CODE(2);
	static const ReturnValue_t INVALID_TIME_FORMAT = MAKE_RETURN_CODE(3);
	static const ReturnValue_t INVALID_DAY_OF_YEAR = MAKE_RETURN_CODE(4);
	static const ReturnValue_t TIME_DOES_NOT_FIT_FORMAT = MAKE_RETURN_CODE(5);

	/**
	 * convert a TimeofDay struct to ccs with seconds resolution
	 *
	 * @param to pointer to a CCS struct
	 * @param from pointer to a TimeOfDay Struct
	 * @return
	 * 		- @c RETURN_OK if OK
	 * 		- @c INVALID_TIMECODE if not OK
	 */
	static ReturnValue_t convertToCcsds(Ccs_seconds *to,
			Clock::TimeOfDay_t const *from);

	/**
	 * Converts to CDS format from timeval.
	 * @param to pointer to the CDS struct to generate
	 * @param from pointer to a timeval struct which comprises a time of day since UNIX epoch.
	 * @return
	 * 		- @c RETURN_OK as it assumes a valid timeval.
	 */
	static ReturnValue_t convertToCcsds(CDS_short* to, timeval const *from);

	static ReturnValue_t convertToCcsds(OBT_FLP* to, timeval const *from);

	/**
	 * convert a TimeofDay struct to ccs with 10E-3 seconds resolution
	 *
	 * The 10E-4 seconds in the CCS Struct are 0 as the TimeOfDay only has ms resolution
	 *
	 * @param to pointer to a CCS struct
	 * @param from pointer to a TimeOfDay Struct
	 * @return
	 * 		- @c RETURN_OK if OK
	 * 		- @c INVALID_TIMECODE if not OK
	 */
	static ReturnValue_t convertToCcsds(Ccs_mseconds *to,
			Clock::TimeOfDay_t const *from);

	/**
	 * SHOULDDO: can this be modified to recognize padding?
	 * Tries to interpret a Level 1 CCSDS time code
	 *
	 * It assumes binary formats contain a valid P Field and recognizes the ASCII format
	 * by the lack of one.
	 *
	 * @param to an empty TimeOfDay struct
	 * @param from pointer to an CCSDS Time code
	 * @param length length of the Time code
	 * @return
	 * 		- @c RETURN_OK if successful
	 * 		- @c UNSUPPORTED_TIME_FORMAT if a (possibly valid) time code is not supported
	 * 		- @c LENGTH_MISMATCH if the length does not match the P Field
	 * 		- @c INVALID_TIME_FORMAT if the format or a value is invalid
	 */
	static ReturnValue_t convertFromCcsds(Clock::TimeOfDay_t *to, uint8_t const *from,
			uint32_t length);

	/**
	 * not implemented yet
	 *
	 * @param to
	 * @param from
	 * @return
	 */
	static ReturnValue_t convertFromCcsds(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCUC(Clock::TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static ReturnValue_t convertFromCUC(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCUC(timeval *to, uint8_t pField,
			uint8_t const *from, uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(timeval *to, uint8_t pField,
			uint8_t const *from, uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCDS(Clock::TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static ReturnValue_t convertFromCDS(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(Clock::TimeOfDay_t *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromASCII(Clock::TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static uint32_t subsecondsToMicroseconds(uint16_t subseconds);
private:
	CCSDSTime();
	virtual ~CCSDSTime();
	/**
	 * checks a ccs time stream for validity
	 *
	 * Stream may be longer than the actual timecode
	 *
	 * @param time pointer to an Ccs stream
	 * @param length length of stream
	 * @return
	 */
	static ReturnValue_t checkCcs(const uint8_t* time, uint8_t length);

	static ReturnValue_t checkTimeOfDay(const Clock::TimeOfDay_t *time);

	static const uint32_t SECONDS_PER_DAY = 24 * 60 * 60;
	static const uint32_t SECONDS_PER_NON_LEAP_YEAR = SECONDS_PER_DAY * 365;
	static const uint32_t DAYS_CCSDS_TO_UNIX_EPOCH = 4383; //!< Time difference between CCSDS and POSIX epoch. This is exact, because leap-seconds where not introduced before 1972.
	static const uint32_t SECONDS_CCSDS_TO_UNIX_EPOCH = DAYS_CCSDS_TO_UNIX_EPOCH
			* SECONDS_PER_DAY;
	/**
	 * @param dayofYear
	 * @param year
	 * @param month
	 * @param day
	 */
	static ReturnValue_t convertDaysOfYear(uint16_t dayofYear, uint16_t year,
			uint8_t *month, uint8_t *day);

	static bool isLeapYear(uint32_t year);
	static ReturnValue_t convertTimevalToTimeOfDay(Clock::TimeOfDay_t* to,
			timeval* from);
};

#endif /* CCSDSTIME_H_ */
