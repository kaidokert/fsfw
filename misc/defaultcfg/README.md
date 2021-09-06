# How to setup configuration folder for FSFW

It is recommended to copy the content of the defaultcfg folder
into a config folder which is in the same directory as the Flight
Software Framework submodule. After that, the config.mk folder should be
included by the primary Makefile with CURRENTPATH set correctly.
