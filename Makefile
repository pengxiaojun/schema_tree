LIBTARGET = libschematree.a

# export OSTYPE
ifeq ($(OSTYPE), cygwin)
CFLAGS += $(GRUTIL_INCS)
CXXFLAGS += -I$(JSON_ROOT)/include
CXXFLAGS += $(GRUTIL_INCS)
else
CFLAGS += $(GRUTIL_INCS) -fPIC
CXXFLAGS += -I$(JSON_ROOT)/include -fPIC
CXXFLAGS += $(GRUTIL_INCS) -fPIC
endif

include Makefile.inc
