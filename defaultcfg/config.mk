CXXSRC += $(wildcard $(CURRENTPATH)/config/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/tmtc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/devices/*.cpp)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/config/
INCLUDES += $(CURRENTPATH)/config/objects
INCLUDES += $(CURRENTPATH)/config/returnvalues
INCLUDES += $(CURRENTPATH)/config/tmtc
INCLUDES += $(CURRENTPATH)/config/events
INCLUDES += $(CURRENTPATH)/config/devices
INCLUDES += $(CURRENTPATH)/config/pollingsequence
INCLUDES += $(CURRENTPATH)/config/ipc
INCLUDES += $(CURRENTPATH)/config/
