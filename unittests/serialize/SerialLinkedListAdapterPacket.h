#ifndef UNITTEST_HOSTED_TESTSERIALLINKEDPACKET_H_
#define UNITTEST_HOSTED_TESTSERIALLINKEDPACKET_H_

#include <fsfw/objectmanager/SystemObjectIF.h>
#include <fsfw/parameters/HasParametersIF.h>
#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerialLinkedListAdapter.h>

#include <cstdint>

class TestPacket : public SerialLinkedListAdapter<SerializeIF> {
 public:
  /**
   * For Deserialization
   */
  TestPacket(const uint8_t* somePacket, size_t size, uint8_t* storePointer, size_t storeSize)
      : buffer(storePointer, storeSize) {
    setLinks();
  }

  /**
   * For Serialization
   */
  TestPacket(uint32_t header, uint32_t tail, const uint8_t* parameters, size_t paramSize)
      : header(header), buffer(parameters, paramSize), tail(tail) {
    setLinks();
  }

  [[nodiscard]] uint32_t getHeader() const { return header.entry; }

  [[nodiscard]] const uint8_t* getBuffer() const { return buffer.entry.getConstBuffer(); }

  size_t getBufferLength() { return buffer.getSerializedSize(); }

  [[nodiscard]] uint16_t getTail() const { return tail.entry; }

 private:
  void setLinks() {
    setStart(&header);
    header.setNext(&buffer);
    buffer.setNext(&tail);
    tail.setEnd();
  }

  SerializeElement<uint32_t> header = 0;
  SerializeElement<SerialBufferAdapter<uint8_t>> buffer;
  SerializeElement<uint32_t> tail = 0;
};

#endif /* UNITTEST_HOSTED_TESTSERIALLINKEDPACKET_H_ */
