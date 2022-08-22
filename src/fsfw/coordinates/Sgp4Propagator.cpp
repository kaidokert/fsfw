#include "fsfw/coordinates/Sgp4Propagator.h"

#include <cstring>

#include "fsfw/coordinates/CoordinateTransformations.h"
#include "fsfw/globalfunctions/constants.h"
#include "fsfw/globalfunctions/math/MatrixOperations.h"
#include "fsfw/globalfunctions/math/VectorOperations.h"
#include "fsfw/globalfunctions/timevalOperations.h"

Sgp4Propagator::Sgp4Propagator() : initialized(false), epoch({0, 0}), whichconst(wgs84) {}

Sgp4Propagator::~Sgp4Propagator() {}

void jday(int year, int mon, int day, int hr, int minute, double sec, double& jd) {
  jd = 367.0 * year - floor((7 * (year + floor((mon + 9) / 12.0))) * 0.25) +
       floor(275 * mon / 9.0) + day + 1721013.5 +
       ((sec / 60.0 + minute) / 60.0 + hr) / 24.0;  // ut in days
                                                    // - 0.5*sgn(100.0*year + mon - 190002.5) + 0.5;
}

void days2mdhms(int year, double days, int& mon, int& day, int& hr, int& minute, double& sec) {
  int i, inttemp, dayofyr;
  double temp;
  int lmonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  dayofyr = (int)floor(days);
  /* ----------------- find month and day of month ---------------- */
  if ((year % 4) == 0) lmonth[1] = 29;

  i = 1;
  inttemp = 0;
  while ((dayofyr > inttemp + lmonth[i - 1]) && (i < 12)) {
    inttemp = inttemp + lmonth[i - 1];
    i++;
  }
  mon = i;
  day = dayofyr - inttemp;

  /* ----------------- find hours minutes and seconds ------------- */
  temp = (days - dayofyr) * 24.0;
  hr = (int)floor(temp);
  temp = (temp - hr) * 60.0;
  minute = (int)floor(temp);
  sec = (temp - minute) * 60.0;
}

ReturnValue_t Sgp4Propagator::initialize(const uint8_t* line1, const uint8_t* line2) {
  char longstr1[130];
  char longstr2[130];

  // need some space for decimal points
  memcpy(longstr1, line1, 69);
  memcpy(longstr2, line2, 69);

  const double deg2rad = Math::PI / 180.0;          //   0.0174532925199433
  const double xpdotp = 1440.0 / (2.0 * Math::PI);  // 229.1831180523293

  double sec, mu, radiusearthkm, tumin, xke, j2, j3, j4, j3oj2;
  int cardnumb, numb, j;
  long revnum = 0, elnum = 0;
  char classification, intldesg[11];
  int year = 0;
  int mon, day, hr, minute, nexp, ibexp;

  getgravconst(whichconst, tumin, mu, radiusearthkm, xke, j2, j3, j4, j3oj2);

  satrec.error = 0;

  // set the implied decimal points since doing a formated read
  // fixes for bad input data values (missing, ...)
  for (j = 10; j <= 15; j++)
    if (longstr1[j] == ' ') longstr1[j] = '_';

  if (longstr1[44] != ' ') longstr1[43] = longstr1[44];
  longstr1[44] = '.';
  if (longstr1[7] == ' ') longstr1[7] = 'U';
  if (longstr1[9] == ' ') longstr1[9] = '.';
  for (j = 45; j <= 49; j++)
    if (longstr1[j] == ' ') longstr1[j] = '0';
  if (longstr1[51] == ' ') longstr1[51] = '0';
  if (longstr1[53] != ' ') longstr1[52] = longstr1[53];
  longstr1[53] = '.';
  longstr2[25] = '.';
  for (j = 26; j <= 32; j++)
    if (longstr2[j] == ' ') longstr2[j] = '0';
  if (longstr1[62] == ' ') longstr1[62] = '0';
  if (longstr1[68] == ' ') longstr1[68] = '0';

  sscanf(longstr1, "%2d %5ld %1c %10s %2d %12lf %11lf %7lf %2d %7lf %2d %2d %6ld ", &cardnumb,
         &satrec.satnum, &classification, intldesg, &satrec.epochyr, &satrec.epochdays,
         &satrec.ndot, &satrec.nddot, &nexp, &satrec.bstar, &ibexp, &numb, &elnum);

  if (longstr2[52] == ' ') {
    sscanf(longstr2, "%2d %5ld %9lf %9lf %8lf %9lf %9lf %10lf %6ld \n", &cardnumb, &satrec.satnum,
           &satrec.inclo, &satrec.nodeo, &satrec.ecco, &satrec.argpo, &satrec.mo, &satrec.no,
           &revnum);
  } else {
    sscanf(longstr2, "%2d %5ld %9lf %9lf %8lf %9lf %9lf %11lf %6ld \n", &cardnumb, &satrec.satnum,
           &satrec.inclo, &satrec.nodeo, &satrec.ecco, &satrec.argpo, &satrec.mo, &satrec.no,
           &revnum);
  }

  // ---- find no, ndot, nddot ----
  satrec.no = satrec.no / xpdotp;  //* rad/min
  satrec.nddot = satrec.nddot * pow(10.0, nexp);
  satrec.bstar = satrec.bstar * pow(10.0, ibexp);

  // ---- convert to sgp4 units ----
  satrec.a = pow(satrec.no * tumin, (-2.0 / 3.0));
  satrec.ndot = satrec.ndot / (xpdotp * 1440.0);  //* ? * minperday
  satrec.nddot = satrec.nddot / (xpdotp * 1440.0 * 1440);

  // ---- find standard orbital elements ----
  satrec.inclo = satrec.inclo * deg2rad;
  satrec.nodeo = satrec.nodeo * deg2rad;
  satrec.argpo = satrec.argpo * deg2rad;
  satrec.mo = satrec.mo * deg2rad;

  satrec.alta = satrec.a * (1.0 + satrec.ecco) - 1.0;
  satrec.altp = satrec.a * (1.0 - satrec.ecco) - 1.0;

  // ----------------------------------------------------------------
  // find sgp4epoch time of element set
  // remember that sgp4 uses units of days from 0 jan 1950 (sgp4epoch)
  // and minutes from the epoch (time)
  // ----------------------------------------------------------------

  // ---------------- temp fix for years from 1957-2056 -------------------
  // --------- correct fix will occur when year is 4-digit in tle ---------
  if (satrec.epochyr < 57) {
    year = satrec.epochyr + 2000;
  } else {
    year = satrec.epochyr + 1900;
  }

  days2mdhms(year, satrec.epochdays, mon, day, hr, minute, sec);
  jday(year, mon, day, hr, minute, sec, satrec.jdsatepoch);

  double unixSeconds = (satrec.jdsatepoch - 2451544.5) * 24 * 3600 + 946684800;

  epoch.tv_sec = unixSeconds;
  double subseconds = unixSeconds - epoch.tv_sec;
  epoch.tv_usec = subseconds * 1000000;

  // ---------------- initialize the orbit at sgp4epoch -------------------
  uint8_t result =
      sgp4init(whichconst, satrec.satnum, satrec.jdsatepoch - 2433281.5, satrec.bstar, satrec.ecco,
               satrec.argpo, satrec.inclo, satrec.mo, satrec.no, satrec.nodeo, satrec);

  if (result != 00) {
    return MAKE_RETURN_CODE(result);
  } else {
    initialized = true;
    return returnvalue::OK;
  }
}

ReturnValue_t Sgp4Propagator::propagate(double* position, double* velocity, timeval time,
                                        uint8_t gpsUtcOffset) {
  if (!initialized) {
    return TLE_NOT_INITIALIZED;
  }

  // Time since epoch in minutes
  timeval timeSinceEpoch = time - epoch;
  double minutesSinceEpoch = timeSinceEpoch.tv_sec / 60. + timeSinceEpoch.tv_usec / 60000000.;

  double yearsSinceEpoch = minutesSinceEpoch / 60 / 24 / 365;

  if ((yearsSinceEpoch > 1) || (yearsSinceEpoch < -1)) {
    return TLE_TOO_OLD;
  }

  double positionTEME[3];
  double velocityTEME[3];

  uint8_t result = sgp4(whichconst, satrec, minutesSinceEpoch, positionTEME, velocityTEME);

  VectorOperations<double>::mulScalar(positionTEME, 1000, positionTEME, 3);
  VectorOperations<double>::mulScalar(velocityTEME, 1000, velocityTEME, 3);

  // Transform to ECF
  double earthRotationMatrix[3][3];
  CoordinateTransformations::getEarthRotationMatrix(time, earthRotationMatrix);

  MatrixOperations<double>::multiply(earthRotationMatrix[0], positionTEME, position, 3, 3, 1);
  MatrixOperations<double>::multiply(earthRotationMatrix[0], velocityTEME, velocity, 3, 3, 1);

  double omegaEarth[3] = {0, 0, Earth::OMEGA};
  double velocityCorrection[3];
  VectorOperations<double>::cross(omegaEarth, position, velocityCorrection);
  VectorOperations<double>::subtract(velocity, velocityCorrection, velocity);

  if (result != 0) {
    return MAKE_RETURN_CODE(result || 0xB0);
  } else {
    return returnvalue::OK;
  }
}
