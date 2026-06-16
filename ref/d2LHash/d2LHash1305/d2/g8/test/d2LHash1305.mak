INCDRS = -I../include/

SRCFLS = ../source/d2LHash1305_const.s 	\
	 ../source/d2LHash1305_maax.S		\
	 ../source/d2LHash1305_keypowers.s	\
	 ../source/d2LHash1305.c 		\
	  ./d2LHash1305_test.c

OBJFLS = ../source/d2LHash1305_const.o 	\
	 ../source/d2LHash1305_maax.o		\
	 ../source/d2LHash1305_keypowers.o	\
	 ../source/d2LHash1305.o 		\
	  ./d2LHash1305_test.o

EXE    = d2LHash1305_test

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
