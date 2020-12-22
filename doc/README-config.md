
## Configuring the FSFW

The FSFW can be configured via the `fsfwconfig` folder. A template folder has
been provided to have a starting point for this. The folder should be added
to the include path.


### Configuring the Event Manager

The number of allowed subscriptions can be modified with the following
parameters:

``` c++
namespace fsfwconfig {
//! Configure the allocated pool sizes for the event manager.
static constexpr size_t FSFW_EVENTMGMR_MATCHTREE_NODES = 240;
static constexpr size_t FSFW_EVENTMGMT_EVENTIDMATCHERS = 120;
static constexpr size_t FSFW_EVENTMGMR_RANGEMATCHERS   = 120;
}
```