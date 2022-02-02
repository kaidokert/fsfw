#ifndef FSFW_DEVICEHANDLERS_DEVICEHANDLERTHERMALSET_H_
#define FSFW_DEVICEHANDLERS_DEVICEHANDLERTHERMALSET_H_

#include "../datapoollocal/LocalPoolVariable.h"
#include "../datapoollocal/StaticLocalDataSet.h"
#include "DeviceHandlerIF.h"

class DeviceHandlerThermalSet : public StaticLocalDataSet<2> {
 public:
  DeviceHandlerThermalSet(
      HasLocalDataPoolIF* hkOwner, uint32_t setId = DeviceHandlerIF::DEFAULT_THERMAL_SET_ID,
      lp_id_t thermalStateId = DeviceHandlerIF::DEFAULT_THERMAL_STATE_POOL_ID,
      lp_id_t heaterRequestId = DeviceHandlerIF::DEFAULT_THERMAL_HEATING_REQUEST_POOL_ID)
      : DeviceHandlerThermalSet(hkOwner->getObjectId(), setId, thermalStateId, heaterRequestId) {}

  DeviceHandlerThermalSet(
      object_id_t deviceHandler, uint32_t setId = DeviceHandlerIF::DEFAULT_THERMAL_SET_ID,
      lp_id_t thermalStateId = DeviceHandlerIF::DEFAULT_THERMAL_STATE_POOL_ID,
      lp_id_t thermalStateRequestId = DeviceHandlerIF::DEFAULT_THERMAL_HEATING_REQUEST_POOL_ID)
      : StaticLocalDataSet(sid_t(deviceHandler, setId)),
        thermalStatePoolId(thermalStateId),
        heaterRequestPoolId(thermalStateRequestId) {}

  const lp_id_t thermalStatePoolId;
  const lp_id_t heaterRequestPoolId;

  lp_var_t<DeviceHandlerIF::dh_thermal_state_t> thermalState =
      lp_var_t<DeviceHandlerIF::dh_thermal_state_t>(thermalStatePoolId, sid.objectId, this);
  lp_var_t<DeviceHandlerIF::dh_heater_request_t> heaterRequest =
      lp_var_t<DeviceHandlerIF::dh_heater_request_t>(heaterRequestPoolId, sid.objectId, this);
};

#endif /* FSFW_DEVICEHANDLERS_DEVICEHANDLERTHERMALSET_H_ */
