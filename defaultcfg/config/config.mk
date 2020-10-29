CXXSRC += $(wildcard $(CURRENTPATH)/config/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/tmtc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/config/devices/*.cpp)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/objects
INCLUDES += $(CURRENTPATH)/returnvalues
INCLUDES += $(CURRENTPATH)/tmtc
INCLUDES += $(CURRENTPATH)/events
INCLUDES += $(CURRENTPATH)/devices
INCLUDES += $(CURRENTPATH)/pollingsequence
INCLUDES += $(CURRENTPATH)/ipc
