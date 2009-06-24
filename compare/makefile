include /cernroot/5_22_00/gcc.3.4.6/test/Makefile.arch

CXXFLAGS += -I/home/pclg-09/vauth/work/MAGE/2009_05_28_newCheckout/MGDO/Base \
	    -I/home/pclg-09/vauth/work/MAGE/2009_05_28_newCheckout/MGDO/Root \
	    -I/home/pclg-09/vauth/work/MAGE/2009_05_28_newCheckout/MGDO/Transforms
LDFLAGS += -L/home/pclg-09/vauth/work/MAGE/2009_05_28_newCheckout/MGDO/lib \
	   -L/home/gx336-01/gsoft/usr/clhep-2.0.3.1/lib

CXXFLAGS +=$(shell clhep-config --include)

LIBS += -lMGDOBase -lMGDORoot -lMGDOTransforms -lCLHEP
GLIBS += -lMGDOBase -lMGDORoot -lMGDOTransforms -lCLHEP

.PHONY: all clean

all: makeTree.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(GLIBS) -o makeTree

clean:
	$(RM) makeTree
