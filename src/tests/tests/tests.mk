CXXSRC += $(wildcard $(CURRENTPATH)/container/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/action/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/serialize/*.cpp)
CXXSRC += $(wildcard $(CURRENTPATH)/storagemanager/*.cpp)

# OSAL not included for now.

INCLUDES += $(CURRENTPATH)