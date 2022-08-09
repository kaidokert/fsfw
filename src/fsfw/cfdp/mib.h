#ifndef FSFW_CFDP_MIB_H
#define FSFW_CFDP_MIB_H

struct IndicationCfg {
  bool eofSentIndicRequired = true;
  bool eofRecvIndicRequired = true;
  bool fileSegmentRecvIndicRequired = true;
  bool transactionFinishedIndicRequired = true;
  bool suspendedIndicRequired = true;
  bool resumedIndicRequired = true;
};

struct LocalEntityCfg {
  cfdp::EntityId localId;
  IndicationCfg indicCfg;
};


#endif  // FSFW_CFDP_MIB_H
