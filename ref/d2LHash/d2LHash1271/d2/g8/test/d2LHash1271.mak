INCDRS = -I../include/

SRCFLS = ../source/d2LHash1271_const.s 	\
	 ../source/d2LHash1271_maax.S		\
	 ../source/d2LHash1271_keypowers.s	\
	 ../source/d2LHash1271.c 		\
	  ./d2LHash1271_test.c

OBJFLS = ../source/d2LHash1271_const.o 	\
	 ../source/d2LHash1271_maax.o		\
	 ../source/d2LHash1271_keypowers.o	\
	 ../source/d2LHash1271.o 		\
	  ./d2LHash1271_test.o

EXE    = d2LHash1271_test

CFLAGS = -march=native -mtune=native -m64 -O3 -funroll-loops -fomit-frame-pointer

CC     = gcc-11
LL     = gcc-11

$(EXE): $(OBJFLS)
	$(LL) -o $@ $(OBJFLS) -lm

.c.o:
	$(CC) $(INCDRS) $(CFLAGS) -o $@ -c $<

clean:
	-rm $(EXE)
	-rm $(OBJFLS)
