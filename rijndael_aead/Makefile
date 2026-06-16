.SECONDARY:

CC=gcc-12
OBJ=obj
ASM=asm
BIN=bin
INC=include
SRC=src
FWOBJ=prebuild

ifndef $(MODE)
MODE:=test
endif



MACRODEFS=

ifdef REPETITIONS
MACRODEFS := $(MACRODEFS) -DREPETITIONS=$(REPETITIONS)
endif
ifdef ITERATIONS
MACRODEFS := $(MACRODEFS) -DITERATIONS=$(ITERATIONS)
endif
ifdef MAXINPUTSIZE
MACRODEFS := $(MACRODEFS) -DMAXINPUTSIZE=$(MAXINPUTSIZE)
endif
ifdef ADLEN
MACRODEFS := $(MACRODEFS) -DADLEN=$(ADLEN)
endif
ifdef STEPSIZE
MACRODEFS := $(MACRODEFS) -DSTEPSIZE=$(STEPSIZE)
endif
ifdef FOLDER
MACRODEFS := $(MACRODEFS) -DFOLDER=\"$(FOLDER)\"
endif
ifdef NAME
MACRODEFS := $(MACRODEFS) -DNAME=\"$(NAME)\"
endif
CCOPTS= -std=c11 -mavx2 -march=native -Wall -Wno-unused-function $(MACRODEFS) # -g -ggdb -fno-omit-frame-pointer -gdwarf-4 #-Wno-unused-variable #  -Werror
BUILDOPT=-O3 #-D DEBUG=1
DEBUGOPT=-O1

include makefiles/rijndael256x.mk

$(OBJ)/%.o: $(ASM)/%.S
	as $(ASM)/$*.S -msyntax=intel -o $(OBJ)/$*.o

dir:
	mkdir -p $(OBJ) $(ASM) $(BIN)

all: all_rijndael256x

clean:
	rm $(OBJ)/*.o
