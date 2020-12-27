CXXSRC += $(wildcard $(CURRENTPATH)/cdatapool/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/ipc/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/objects/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/pollingsequence/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/events/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/*.cpp)

INCLUDES += $(CURRENTPATH)
