CXXSRC += $(wildcard $(CURRENTPATH)/cdatapool/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/events/*.cpp)

INCLUDES += $(CURRENTPATH)
INCLUDES += $(CURRENTPATH)/objects
INCLUDES += $(CURRENTPATH)/ipc
INCLUDES += $(CURRENTPATH)/pollingsequence
INCLUDES += $(CURRENTPATH)/returnvalues
INCLUDES += $(CURRENTPATH)/tmtc
INCLUDES += $(CURRENTPATH)/events
INCLUDES += $(CURRENTPATH)/devices
INCLUDES += $(CURRENTPATH)/cdatapool
