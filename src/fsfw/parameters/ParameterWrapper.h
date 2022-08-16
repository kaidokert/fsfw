#ifndef FSFW_PARAMETERS_PARAMETERWRAPPER_H_
#define FSFW_PARAMETERS_PARAMETERWRAPPER_H_

#include <cstddef>

#include "../globalfunctions/Type.h"
#include "../returnvalues/returnvalue.h"
#include "../serialize/SerializeAdapter.h"
#include "../serialize/SerializeIF.h"

/**
 * @brief       This wrapper encapsulates the access to parameters provided by HasParametersIF.
 * @details
 * This wrapper is used by the ParameterHelper to interface with the on-board parameters
 * exposed by the software via the HasParametersIF. A handle of this wrapper is passed
 * to the user which then can be used to set or dump the parameters.
 *
 * The wrapper provides a set of setter functions. The user should call those setter functions,
 * supplying an address to the local parameters. The user can also deserialize or
 * serialize the parameter data. Please note that this will also serialize and deserialize
 * the parameter information field (4 bytes) containing the ECSS PTC, PFC and rows and columns
 * number.
 */
class ParameterWrapper : public SerializeIF {
  friend class DataPoolParameterWrapper;

 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::PARAMETER_WRAPPER;
  static const ReturnValue_t UNKNOWN_DATATYPE = MAKE_RETURN_CODE(0x01);
  static const ReturnValue_t DATATYPE_MISSMATCH = MAKE_RETURN_CODE(0x02);
  static const ReturnValue_t READONLY = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t TOO_BIG = MAKE_RETURN_CODE(0x04);
  static const ReturnValue_t SOURCE_NOT_SET = MAKE_RETURN_CODE(0x05);
  static const ReturnValue_t OUT_OF_BOUNDS = MAKE_RETURN_CODE(0x06);
  static const ReturnValue_t NOT_SET = MAKE_RETURN_CODE(0x07);
  static const ReturnValue_t COLUMN_OR_ROWS_ZERO = MAKE_RETURN_CODE(0x08);

  ParameterWrapper();
  ParameterWrapper(Type type, uint8_t rows, uint8_t columns, void *data);
  ParameterWrapper(Type type, uint8_t rows, uint8_t columns, const void *data);
  virtual ~ParameterWrapper();

  virtual ReturnValue_t serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                  Endianness streamEndianness) const override;

  virtual size_t getSerializedSize() const override;

  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness) override;

  virtual ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                                    Endianness streamEndianness, uint16_t startWritingAtIndex = 0);

  /**
   * Get a specific parameter value by supplying the row and the column.
   * @tparam T Type of target data
   * @param value [out] Pointer to storage location
   * @param row
   * @param column
   * @return
   * -@c returnvalue::OK if element was retrieved successfully
   * -@c NOT_SET data has not been set yet
   * -@c DATATYPE_MISSMATCH Invalid supplied type
   * -@c OUT_OF_BOUNDS Invalid row and/or column.
   */
  template <typename T>
  ReturnValue_t getElement(T *value, uint8_t row = 0, uint8_t column = 0) const;

  template <typename T>
  void set(T *data, uint8_t rows, uint8_t columns) {
    this->data = data;
    this->readonlyData = data;
    this->type = PodTypeConversion<T>::type;
    this->rows = rows;
    this->columns = columns;
    this->pointsToStream = false;
  }

  template <typename T>
  void set(const T *readonlyData, uint8_t rows, uint8_t columns) {
    this->data = nullptr;
    this->readonlyData = readonlyData;
    this->type = PodTypeConversion<T>::type;
    this->rows = rows;
    this->columns = columns;
    this->pointsToStream = false;
  }

  /**
   * Setter function for scalar non-const entries
   * @tparam T
   * @param member
   */
  template <typename T>
  void set(T &member) {
    this->set(&member, 1, 1);
  }

  /**
   * Setter function for scalar const entries.
   * TODO: This is confusing, it should not be called set. Maybe we should call all functions
   * assign instead?
   * @tparam T
   * @param readonlyMember
   */
  template <typename T>
  void set(const T &readonlyMember) {
    this->set(&readonlyMember, 1, 1);
  }

  template <typename T>
  void setVector(T &member) {
    /* For a vector entry, the number of rows will be one
    (left to right, top to bottom indexing) */
    this->set(member, 1, sizeof(member) / sizeof(member[0]));
  }

  template <typename T>
  void setVector(const T &member) {
    /* For a vector entry, the number of rows will be one
    (left to right, top to bottom indexing) */
    this->set(member, 1, sizeof(member) / sizeof(member[0]));
  }
  template <typename T>
  void setMatrix(T &member) {
    this->set(member[0], sizeof(member) / sizeof(member[0]),
              sizeof(member[0]) / sizeof(member[0][0]));
  }

  template <typename T>
  void setMatrix(const T &member) {
    this->set(member[0], sizeof(member) / sizeof(member[0]),
              sizeof(member[0]) / sizeof(member[0][0]));
  }

  ReturnValue_t set(Type type, uint8_t rows, uint8_t columns, const void *data, size_t dataSize);

  ReturnValue_t set(const uint8_t *stream, size_t streamSize,
                    const uint8_t **remainingStream = nullptr, size_t *remainingSize = nullptr);

  ReturnValue_t copyFrom(const ParameterWrapper *from, uint16_t startWritingAtIndex);

 private:
  void convertLinearIndexToRowAndColumn(uint16_t index, uint8_t *row, uint8_t *column);

  uint16_t convertRowAndColumnToLinearIndex(uint8_t row, uint8_t column);

  bool pointsToStream = false;

  Type type;
  uint8_t rows = 0;
  uint8_t columns = 0;
  void *data = nullptr;
  const void *readonlyData = nullptr;

  template <typename T>
  ReturnValue_t serializeData(uint8_t **buffer, size_t *size, size_t maxSize,
                              Endianness streamEndianness) const;

  template <typename T>
  ReturnValue_t deSerializeData(uint8_t startingRow, uint8_t startingColumn, const void *from,
                                uint8_t fromRows, uint8_t fromColumns);
};

template <typename T>
inline ReturnValue_t ParameterWrapper::getElement(T *value, uint8_t row, uint8_t column) const {
  if (readonlyData == nullptr) {
    return NOT_SET;
  }

  if (PodTypeConversion<T>::type != type) {
    return DATATYPE_MISSMATCH;
  }

  if ((row >= rows) or (column >= columns)) {
    return OUT_OF_BOUNDS;
  }

  if (pointsToStream) {
    const uint8_t *streamWithType = static_cast<const uint8_t *>(readonlyData);
    streamWithType += (row * columns + column) * type.getSize();
    size_t size = type.getSize();
    return SerializeAdapter::deSerialize(value, &streamWithType, &size,
                                         SerializeIF::Endianness::BIG);
  } else {
    const T *dataWithType = static_cast<const T *>(readonlyData);
    *value = dataWithType[row * columns + column];
    return returnvalue::OK;
  }
}

#endif /* FSFW_PARAMETERS_PARAMETERWRAPPER_H_ */
