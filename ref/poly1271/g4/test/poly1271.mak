INCDRS = -I../include/

SRCFLS = ../source/poly1271_const.S 		\
	 ../source/poly1271_maax.S		\
	 ../source/poly1271_keypowers.S	\
	 ../source/poly1271.c 			\
	  ./poly1271_test.c

OBJFLS = ../source/poly1271_const.o 		\
	 ../source/poly1271_maax.o		\
	 ../source/poly1271_keypowers.o	\
	 ../source/poly1271.o 			\
	  ./poly1271_test.o

EXE    = poly1271_test

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
