#ifndef FSFW_INTERNALERROR_INTERNALERRORDATASET_H_
#define FSFW_INTERNALERROR_INTERNALERRORDATASET_H_

#include <fsfw/datapoollocal/LocalPoolVariable.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>

enum errorPoolIds { TM_HITS, QUEUE_HITS, STORE_HITS };

class InternalErrorDataset : public StaticLocalDataSet<3 * sizeof(uint32_t)> {
 public:
  static constexpr uint8_t ERROR_SET_ID = 0;

  InternalErrorDataset(HasLocalDataPoolIF* owner) : StaticLocalDataSet(owner, ERROR_SET_ID) {}

  InternalErrorDataset(object_id_t objectId) : StaticLocalDataSet(sid_t(objectId, ERROR_SET_ID)) {}

  lp_var_t<uint32_t> tmHits = lp_var_t<uint32_t>(sid.objectId, TM_HITS, this);
  lp_var_t<uint32_t> queueHits = lp_var_t<uint32_t>(sid.objectId, QUEUE_HITS, this);
  lp_var_t<uint32_t> storeHits = lp_var_t<uint32_t>(sid.objectId, STORE_HITS, this);
};

#endif /* FSFW_INTERNALERROR_INTERNALERRORDATASET_H_ */
