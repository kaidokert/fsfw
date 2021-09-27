#ifndef FSFW_SRC_FSFW_COORDINATES_COORDINATESCONF_H_
#define FSFW_SRC_FSFW_COORDINATES_COORDINATESCONF_H_

#include "fsfw/FSFW.h"

#ifndef FSFW_ADD_COORDINATES
#warning Coordinates files were included but compilation was \
    not enabled with FSFW_ADD_COORDINATES
#endif

#ifndef FSFW_ADD_SGP4_PROPAGATOR
#warning Coordinates files were included but SGP4 contributed code compilation was \
    not enabled with FSFW_ADD_SGP4_PROPAGATOR
#endif

#endif /* FSFW_SRC_FSFW_COORDINATES_COORDINATESCONF_H_ */
