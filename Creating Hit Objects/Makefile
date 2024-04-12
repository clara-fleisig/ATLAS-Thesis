# Makefile
CPP = g++ -g

OBJDIR = obj
SRCDIR = src
BINDIR = bin

ROOTCXXFLAGS  = $(shell root-config --cflags)
CPPFLAGS = -std=c++17 -Wall -c $(ROOTCXXFLAGS) 
LDFLAGS = 
ROOTFITLIBS = -lRooFitCore -lRooFit -lRooStats -lCore
ROOTLIBS    = $(shell root-config --libs) -lCore
LDLIBS =  $(ROOTLIBS)


default: build ${BINDIR}/HitFunctions 

build:
	@mkdir -p obj
	@mkdir -p bin

${BINDIR}/HitFunctions: ${OBJDIR}/HitFunctions.o 
	${CPP} -o ${BINDIR}/HitFunctions ${OBJDIR}/HitFunctions.o ${LDLIBS}

${OBJDIR}/%.o: ${SRCDIR}/%.cxx ${SRCDIR}/%.h
	${CPP} ${CPPFLAGS} -c -o $@ $<

${OBJDIR}/%.o: ${SRCDIR}/%.cxx
	${CPP} ${CPPFLAGS} -c -o $@ $<

clean:
	rm -f ${OBJDIR}/*.o *.o ${BINDIR}/*

