/*
 * CCSDSTimeHelper.h
 *
 *  Created on: 22.03.2013
 *      Author: tod
 */

#ifndef CCSDSTIME_H_
#define CCSDSTIME_H_

#include <framework/osal/OSAL.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <stdint.h>

bool operator<(const timeval& lhs, const timeval& rhs);
bool operator==(const timeval& lhs, const timeval& rhs);
/**
 * static helper class for CCSDS Time Code Formats
 *
 * as described in CCSDS 301.0-B-3
 *
 * Still work in progress thus TODO finishme
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
	 * Struct for the CCS fromat in day of month variation with seconds resolution
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

	static const uint8_t INTERFACE_ID = CCSDS_TIME_HELPER_CLASS;
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
	 * Assumes a valid TimeOfDay. TODO: maybe check it anyway?
	 *
	 * @param to pointer to a CCS struct
	 * @param from pointer to a TimeOfDay Struct
	 * @return
	 * 		- @c RETURN_OK as it assumes a valid TimeOfDay
	 */
	static ReturnValue_t convertToCcsds(Ccs_seconds *to,
			TimeOfDay_t const *from);

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
	 * Assumes a valid TimeOfDay. TODO: maybe check it anyway?
	 *
	 * The 10E-4 seconds in the CCS Struct are 0 as the TimeOfDay only has ms resolution
	 *
	 * @param to pointer to a CCS struct
	 * @param from pointer to a TimeOfDay Struct
	 * @return
	 * 		- @c RETURN_OK as it assumes a valid TimeOfDay
	 */
	static ReturnValue_t convertToCcsds(Ccs_mseconds *to,
			TimeOfDay_t const *from);

	/**
	 * TODO: can this be modified to recognize padding?
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
	 * 		- @c UNSUPPORTED_TIME_FORMAT if a (possibly valid) time code is not supported TODO: the missing codes should be implemented...
	 * 		- @c LENGTH_MISMATCH if the length does not match the P Field
	 * 		- @c INVALID_TIME_FORMAT if the format or a value is invalid
	 */
	static ReturnValue_t convertFromCcsds(TimeOfDay_t *to, uint8_t const *from,
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

	static ReturnValue_t convertFromCUC(TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static ReturnValue_t convertFromCUC(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCUC(timeval *to, uint8_t pField,
			uint8_t const *from, uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(timeval *to, uint8_t pField,
			uint8_t const *from, uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCDS(TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static ReturnValue_t convertFromCDS(timeval *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromCCS(TimeOfDay_t *to, uint8_t const *from,
			uint32_t* foundLength, uint32_t maxLength);

	static ReturnValue_t convertFromASCII(TimeOfDay_t *to, uint8_t const *from,
			uint8_t length);

	static uint32_t subsecondsToMicroseconds(uint16_t subseconds);
private:
	CCSDSTime();
	virtual ~CCSDSTime();
	/**
	 * checks a ccs time struct for validity
	 *
	 * only checks year to second, subseconds must be checked elsewhere
	 *
	 * @param time pointer to an Ccs struct (should be cast to Ccs_seconds as subseconds are not checked. Cast is save as subseconds are at the end of the struct)
	 * @return
	 */
	static ReturnValue_t checkCcs(Ccs_seconds *time);

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

	static bool isLeapYear(uint16_t year);
	static ReturnValue_t convertTimevalToTimeOfDay(TimeOfDay_t* to,
			timeval* from);
};

#endif /* CCSDSTIME_H_ */
