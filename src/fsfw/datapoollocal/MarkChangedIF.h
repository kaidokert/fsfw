#ifndef FSFW_DATAPOOLLOCAL_MARKCHANGEDIF_H_
#define FSFW_DATAPOOLLOCAL_MARKCHANGEDIF_H_

/**
 * Common interface for local pool entities which can be marked as changed.
 */
class MarkChangedIF {
 public:
  virtual ~MarkChangedIF(){};

  virtual bool hasChanged() const = 0;
  virtual void setChanged(bool changed) = 0;
};

#endif /* FSFW_DATAPOOLLOCAL_MARKCHANGEDIF_H_ */
