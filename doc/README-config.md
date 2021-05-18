Configuring the FSFW
======

The FSFW can be configured via the `fsfwconfig` folder. A template folder has
been provided to have a starting point for this. The folder should be added
to the include path. The primary configuration file is the `FSFWConfig.h` folder. Some
of the available options will be explained in more detail here.

# Auto-Translation of Events

The FSFW allows the automatic translation of events, which allows developers to track triggered
events directly via consoke output. Using this feature requires:

1. `FSFW_OBJ_EVENT_TRANSLATION` set to 1 in the configuration file.
2. Special auto-generated translation files which translate event IDs and object IDs into
   human readable strings. These files can be generated using the 
   [modgen Python scripts](https://git.ksat-stuttgart.de/source/modgen.git).
3. The generated translation files for the object IDs should be named `translatesObjects.cpp`
   and `translateObjects.h` and should be copied to the `fsfwconfig/objects` folder
4. The generated translation files for the event IDs should be named `translateEvents.cpp` and
   `translateEvents.h` and should be copied to the `fsfwconfig/events` folder
   
An example implementations of these translation file generators can be found as part
of the [SOURCE project here](https://git.ksat-stuttgart.de/source/sourceobsw/-/tree/development/generators)
or the [FSFW example](https://egit.irs.uni-stuttgart.de/fsfw/fsfw_example_public/src/branch/master/generators)

## Configuring the Event Manager

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

