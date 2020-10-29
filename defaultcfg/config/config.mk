CXXSRC += $(wildcard $(CURRENTPATH)/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/tmtc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/devices/*.cpp)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/objects
INCLUDES += $(CURRENTPATH)/returnvalues
INCLUDES += $(CURRENTPATH)/tmtc
INCLUDES += $(CURRENTPATH)/events
INCLUDES += $(CURRENTPATH)/devices
INCLUDES += $(CURRENTPATH)/pollingsequence
INCLUDES += $(CURRENTPATH)/ipc
