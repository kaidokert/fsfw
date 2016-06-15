#!/bin/bash
#
# OPUS makefile
#
#  Created on: Mar 04, 2010
#      Author: ziemke
#      Author: Claas Ziemke
#      Copyright 2010, Claas Ziemke <claas.ziemke@gmx.net>
#

BASEDIR=../../
include $(BASEDIR)/options.mk

OBJ = 	$(BUILDDIR)/OPUSDataPool.o \
		$(BUILDDIR)/OPUSDataPoolItem.o \
		$(BUILDDIR)/OPUSDataSet.o \
		$(BUILDDIR)/OPUSRegVar.o

all: $(OBJ)
	 		
$(BUILDDIR)/%.o: %.cpp %.h
	$(CPP) $(CFLAGS) $(DEFINES) $(CCOPT) ${INCLUDE} -c $< -o $@
	
clean:
	$(RM) *.o *.gcno *.gcda
