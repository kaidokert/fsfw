#ifndef FSFW_RMAP_RMAPSTRUCTS_H_
#define FSFW_RMAP_RMAPSTRUCTS_H_

#include <cstdint>

#include "rmapConf.h"

// SHOULDDO: having the defines within a namespace would be nice. Problem are the defines
// referencing the previous define, eg RMAP_COMMAND_WRITE

//////////////////////////////////////////////////////////////////////////////////
// RMAP command bits
//#define RMAP_COMMAND_BIT_INCREMENT	2
//#define RMAP_COMMAND_BIT_REPLY		3
//#define RMAP_COMMAND_BIT_WRITE		5
//#define RMAP_COMMAND_BIT_VERIFY		4
//#define RMAP_COMMAND_BIT			6

namespace RMAPIds {

static const uint8_t RMAP_COMMAND_BIT_INCREMENT = 2;
static const uint8_t RMAP_COMMAND_BIT_REPLY = 3;
static const uint8_t RMAP_COMMAND_BIT_WRITE = 5;
static const uint8_t RMAP_COMMAND_BIT_VERIFY = 4;
static const uint8_t RMAP_COMMAND_BIT = 6;

//////////////////////////////////////////////////////////////////////////////////
// RMAP commands
static const uint8_t RMAP_COMMAND_WRITE =
    ((1 << RMAP_COMMAND_BIT) | (1 << RMAP_COMMAND_BIT_WRITE) | (1 << RMAP_COMMAND_BIT_REPLY));
static const uint8_t RMAP_COMMAND_READ = ((1 << RMAP_COMMAND_BIT) | (1 << RMAP_COMMAND_BIT_REPLY));
static const uint8_t RMAP_REPLY_WRITE =
    ((1 << RMAP_COMMAND_BIT_WRITE) | (1 << RMAP_COMMAND_BIT_REPLY));
static const uint8_t RMAP_REPLY_READ = ((1 << RMAP_COMMAND_BIT_REPLY));
//#define RMAP_COMMAND_WRITE			((1<<RMAP_COMMAND_BIT) | (1<<RMAP_COMMAND_BIT_WRITE)
//| (1<<RMAP_COMMAND_BIT_REPLY)) #define RMAP_COMMAND_WRITE_VERIFY	((1<<RMAP_COMMAND_BIT) |
//(1<<RMAP_COMMAND_BIT_WRITE) | (1<<RMAP_COMMAND_BIT_REPLY) | (1<<RMAP_COMMAND_BIT_VERIFY)) #define
// RMAP_COMMAND_READ			((1<<RMAP_COMMAND_BIT) | (1<<RMAP_COMMAND_BIT_REPLY))

//#define RMAP_REPLY_WRITE			((1<<RMAP_COMMAND_BIT_WRITE) |
//(1<<RMAP_COMMAND_BIT_REPLY))
//#define RMAP_REPLY_WRITE_VERIFY		((1<<RMAP_COMMAND_BIT_WRITE) |
//(1<<RMAP_COMMAND_BIT_REPLY) | (1<<RMAP_COMMAND_BIT_VERIFY)) #define RMAP_REPLY_READ
//((1<<RMAP_COMMAND_BIT_REPLY))

//////////////////////////////////////////////////////////////////////////////////
// useful info
static const uint8_t RMAP_COMMAND_HEADER_LEN = 16;
static const uint8_t RMAP_WRITE_REPLY_HEADER_LEN = 8;
static const uint8_t RMAP_READ_REPLY_HEADER_LEN = 12;
static const uint8_t RMAP_DATA_FOOTER_SIZE = 1;  // SIZE OF CRC
//#define RMAP_COMMAND_HEADER_LEN		16
//#define RMAP_WRITE_REPLY_HEADER_LEN	8
//#define RMAP_READ_REPLY_HEADER_LEN	12

}  // namespace RMAPIds

namespace RMAPStructs {
struct rmap_cmd_header {
  uint8_t dest_address;
  uint8_t protocol;
  uint8_t instruction;
  uint8_t dest_key;
  uint8_t source_address;
  uint8_t tid_h;
  uint8_t tid_l;
  uint8_t extended_address;
  uint8_t address_hh;
  uint8_t address_h;
  uint8_t address_l;
  uint8_t address_ll;
  uint8_t datalen_h;
  uint8_t datalen_m;
  uint8_t datalen_l;
  uint8_t header_crc;
};

struct rmap_read_reply_header {
  uint8_t dest_address;
  uint8_t protocol;
  uint8_t instruction;
  uint8_t status;
  uint8_t source_address;
  uint8_t tid_h;
  uint8_t tid_l;
  uint8_t reserved;
  uint8_t datalen_h;
  uint8_t datalen_m;
  uint8_t datalen_l;
  uint8_t header_crc;
};

struct rmap_write_reply_header {
  uint8_t dest_address;
  uint8_t protocol;
  uint8_t instruction;
  uint8_t status;
  uint8_t source_address;
  uint8_t tid_h;
  uint8_t tid_l;
  uint8_t header_crc;
};

}  // namespace RMAPStructs

#endif /* FSFW_RMAP_RMAPSTRUCTS_H_ */
