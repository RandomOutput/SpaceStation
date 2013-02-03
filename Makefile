APP = stars

include $(SDK_DIR)/Makefile.defs

OBJS = $(ASSETS).gen.o main.o
ASSETDEPS += images/*.png $(ASSETS).lua

include $(SDK_DIR)/Makefile.rules
