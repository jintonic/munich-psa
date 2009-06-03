include /cernroot/5_22_00/gcc.3.4.6/test/Makefile.arch

CXXFLAGS += -I/remote/pclg-13/jingliu/work/MGDO/Base \
	    -I/remote/pclg-13/jingliu/work/MGDO/Root \
	    -I/remote/pclg-13/jingliu/work/MGDO/Transforms
LDFLAGS += -L/remote/pclg-13/jingliu/work/MGDO/lib \
	   -L/remote/pclg-05/jingliu/usr/lib

CXXFLAGS +=$(shell clhep-config --include)

LIBS += -lMGDOBase -lMGDORoot -lMGDOTransforms -lCLHEP
GLIBS += -lMGDOBase -lMGDORoot -lMGDOTransforms -lCLHEP

.PHONY: all clean

all: psb.cc psbDict.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(GLIBS) -o psb

psbDict.cc: psb.hh psbLinkDef.hh
	rootcint -f $@ -c $^

clean:
	$(RM) psbDict.cc psb
