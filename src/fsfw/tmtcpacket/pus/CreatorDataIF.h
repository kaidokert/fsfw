#ifndef FSFW_TMTCPACKET_CREATORDATAIF_H
#define FSFW_TMTCPACKET_CREATORDATAIF_H

#include "definitions.h"

class CreatorDataIF {
 public:
  virtual ecss::DataWrapper& getDataWrapper() = 0;
};
#endif  // FSFW_TMTCPACKET_CREATORDATAIF_H
