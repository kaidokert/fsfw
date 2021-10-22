#ifndef COMMON_GPIO_GPIODEFINITIONS_H_
#define COMMON_GPIO_GPIODEFINITIONS_H_

#include <string>
#include <unordered_map>
#include <map>

using gpioId_t = uint16_t;

namespace gpio {

enum Levels: uint8_t {
    LOW = 0,
    HIGH = 1,
    NONE = 99
};

enum Direction: uint8_t {
    IN = 0,
    OUT = 1
};

enum GpioOperation {
    READ,
    WRITE
};

enum class GpioTypes {
    NONE,
    GPIO_REGULAR_BY_CHIP,
    GPIO_REGULAR_BY_LABEL,
    GPIO_REGULAR_BY_LINE_NAME,
    CALLBACK
};

static constexpr gpioId_t NO_GPIO = -1;

using gpio_cb_t = void (*) (gpioId_t gpioId, gpio::GpioOperation gpioOp, gpio::Levels value,
        void* args);

}

/**
 * @brief   Struct containing information about the GPIO to use. This is
 *          required by the libgpiod to access and drive a GPIO.
 * @param chipname      String of the chipname specifying the group which contains the GPIO to
 *                      access. E.g. gpiochip0. To detect names of GPIO groups run gpiodetect on
 *                      the linux command line.
 * @param lineNum       The offset of the GPIO within the GPIO group.
 * @param consumer      Name of the consumer. Simply a description of the GPIO configuration.
 * @param direction     Specifies whether the GPIO should be used as in- or output.
 * @param initValue     Defines the initial state of the GPIO when configured as output.
 *                      Only required for output GPIOs.
 * @param lineHandle    The handle returned by gpiod_chip_get_line will be later written to this
 *                      pointer.
 */
class GpioBase {
public:

    GpioBase() = default;

    GpioBase(gpio::GpioTypes gpioType, std::string consumer, gpio::Direction direction,
            gpio::Levels initValue):
            gpioType(gpioType), consumer(consumer),direction(direction), initValue(initValue) {}

    virtual~ GpioBase() {};

    // Can be used to cast GpioBase to a concrete child implementation
    gpio::GpioTypes gpioType = gpio::GpioTypes::NONE;
    std::string consumer;
    gpio::Direction direction = gpio::Direction::IN;
    gpio::Levels initValue = gpio::Levels::NONE;
};

class GpiodRegularBase: public GpioBase {
public:
    GpiodRegularBase(gpio::GpioTypes gpioType, std::string consumer, gpio::Direction direction,
            gpio::Levels initValue, int lineNum):
            GpioBase(gpioType, consumer, direction, initValue), lineNum(lineNum) {
    }

    // line number will be configured at a later point for the open by line name configuration
    GpiodRegularBase(gpio::GpioTypes gpioType, std::string consumer, gpio::Direction direction,
            gpio::Levels initValue): GpioBase(gpioType, consumer, direction, initValue) {
    }

    int lineNum = 0;
    struct gpiod_line* lineHandle = nullptr;
};

class GpiodRegularByChip: public GpiodRegularBase {
public:
    GpiodRegularByChip() :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_CHIP,
                    std::string(), gpio::Direction::IN, gpio::LOW, 0) {
    }

    GpiodRegularByChip(std::string chipname_, int lineNum_, std::string consumer_,
            gpio::Direction direction_, gpio::Levels initValue_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_CHIP,
                    consumer_, direction_, initValue_, lineNum_),
            chipname(chipname_){
    }

    GpiodRegularByChip(std::string chipname_, int lineNum_, std::string consumer_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_CHIP, consumer_,
                    gpio::Direction::IN, gpio::LOW, lineNum_),
            chipname(chipname_) {
    }

    std::string chipname;
};

class GpiodRegularByLabel: public GpiodRegularBase {
public:
    GpiodRegularByLabel(std::string label_, int lineNum_, std::string consumer_,
            gpio::Direction direction_, gpio::Levels initValue_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_LABEL, consumer_,
                    direction_, initValue_, lineNum_),
            label(label_) {
    }

    GpiodRegularByLabel(std::string label_, int lineNum_, std::string consumer_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_LABEL, consumer_,
                    gpio::Direction::IN, gpio::LOW, lineNum_),
            label(label_) {
    }

    std::string label;
};

/**
 * @brief   Passing this GPIO configuration to the GPIO IF object will try to open the GPIO by its
 *          line name. This line name can be set in the device tree and must be unique. Otherwise
 *          the driver will open the first line with the given name.
 */
class GpiodRegularByLineName: public GpiodRegularBase {
public:
    GpiodRegularByLineName(std::string lineName_, std::string consumer_, gpio::Direction direction_,
            gpio::Levels initValue_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME, consumer_, direction_,
                    initValue_), lineName(lineName_) {
    }

    GpiodRegularByLineName(std::string lineName_, std::string consumer_) :
            GpiodRegularBase(gpio::GpioTypes::GPIO_REGULAR_BY_LINE_NAME, consumer_,
                    gpio::Direction::IN, gpio::LOW), lineName(lineName_) {
    }

    std::string lineName;
};

class GpioCallback: public GpioBase {
public:
    GpioCallback(std::string consumer, gpio::Direction direction_, gpio::Levels initValue_,
            gpio::gpio_cb_t callback, void* callbackArgs):
            GpioBase(gpio::GpioTypes::CALLBACK, consumer, direction_, initValue_),
            callback(callback), callbackArgs(callbackArgs) {}

    gpio::gpio_cb_t callback = nullptr;
    void* callbackArgs = nullptr;
};


using GpioMap = std::map<gpioId_t, GpioBase*>;
using GpioUnorderedMap = std::unordered_map<gpioId_t, GpioBase*>;
using GpioMapIter = GpioMap::iterator;
using GpioUnorderedMapIter = GpioUnorderedMap::iterator;

#endif /* LINUX_GPIO_GPIODEFINITIONS_H_ */
