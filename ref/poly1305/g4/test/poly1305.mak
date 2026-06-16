INCDRS = -I../include/

SRCFLS = ../source/poly1305_const.s 		\
	 ../source/poly1305_maax.S		\
	 ../source/poly1305_keypowers.s	\
	 ../source/poly1305.c 			\
	  ./poly1305_test.c

OBJFLS = ../source/poly1305_const.o 		\
	 ../source/poly1305_maax.o		\
	 ../source/poly1305_keypowers.o	\
	 ../source/poly1305.o 			\
	  ./poly1305_test.o

EXE    = poly1305_test

CFLAGS = -march=native -mtune=native -m64 -O3 -funroll-loops -fomit-frame-pointer

CC     = gcc-10
LL     = gcc-10

$(EXE): $(OBJFLS)
	$(LL) -o $@ $(OBJFLS) -lm

.c.o:
	$(CC) $(INCDRS) $(CFLAGS) -o $@ -c $<

clean:
	-rm $(EXE)
	-rm $(OBJFLS)
