CC = gcc
BINDIR = bin
SRCDIR = src
_OBJDIR = obj
_ASMDIR = asm
TESTRESDIR = results
PLOTDIR = plots
OBJDIR = $(_OBJDIR)/$(BINNAME)
ASMDIR = $(_ASMDIR)/$(BINNAME)
TESTDIR= tests
INCDIRS=-Iinclude -I/opt/local/include -I/local/openssl3/include -Ihacl_include -I/usr/include
LIBDIRS= -L/opt/local/lib -Llib/ -L/usr/lib #-L/user/local/ssl/lib64

_DEFS=-DKEYINCLUDE_H=\"$(KEYINCLUDE).h\" -DKEYTRANSFORM=$(KEYTRANSFORM)\
     -DMSGINCLUDE_H=\"$(MSGINCLUDE).h\" -DMSGTRANSFORM=$(MSGTRANSFORM)\
	 -DFIELDELEMINCLUDE_H=\"$(FIELDELEMINCLUDE).h\" -DFIELDELEMTRANSFORM=$(FIELDELEMTRANSFORM)\
	 -DOUTERPOLY_H=\"$(OUTERPOLY_H)\" -DOUTERPOLY=$(OUTERPOLY) $(MDEFS) -DFOLDER=\"$(BENCHDIR)\" -DNAME=\"$(BENCHRESNAME)\"
__DEPS =  $(OBJDIR)/$(OUTERPOLY).o $(OBJDIR)/$(KEYINCLUDE).o $(OBJDIR)/$(MSGINCLUDE).o $(OBJDIR)/$(FIELDELEMINCLUDE).o
_DEPS = $(__DEPS) $(OBJDIR)/hash.o $(OBJDIR)/key_expansion.o $(OBJDIR)/randombytes.o
REFDEPS = $(OBJDIR)/$(BINNAME)_hash.o $(OBJDIR)/randombytes.o

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

override _DEPS := $(_DEPS) $(ADDDEPS)
override CCFLAGS := $(CCFLAGS) -std=c11 -Wall -fstack-protector -ggdb -Werror -fpic -save-temps=obj

ifdef USE_OPEN_SSL
	override _DEFS := $(_DEFS) -DUSE_OPEN_SSL
	override LDFLAGS := $(LDFLAGS) -lssl -lcrypto
else
ifdef USE_HACL
	override _DEFS := $(_DEFS) -D_DEFAULT_SOURCE
	override LDFLAGS := $(LDFLAGS) -l:libevercrypt.a
	override CCFLAGS := $(CCFLAGS) -Wno-discarded-qualifiers
endif
	override LDFLAGS := $(LDFLAGS) -lsodium
endif
ifdef USE_CTGRIND
	override _DEFS := $(_DEFS) -DUSE_CTGRIND
	override LDFLAGS := $(LDFLAGS) -lctgrind
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	override CCFLAGS := $(CCFLAGS) -Wno-error=unused-command-line-argument -Wno-unused-command-line-argument
else
CC_V := $(shell $(CC) --version)
ifeq ($(findstring clang,$(CC_V)),clang)
	override CCFLAGS := $(CCFLAGS) -Wno-error=unused-command-line-argument -Wno-unused-command-line-argument
endif
endif

ifdef INNERPOLY
DEPS = $(_DEPS) $(OBJDIR)/$(INNERPOLY).o
DEFS = $(_DEFS) -DINNERPOLY_H=\"$(INNERPOLY_H)\" -DINNERPOLY=$(INNERPOLY)
else
DEPS = $(_DEPS)
DEFS = $(_DEFS)
endif

dir:
	mkdir -p $(OBJDIR) $(BINDIR) $(ASMDIR) $(BENCHDIR) $(TESTRESDIR) $(PLOTDIR)

$(ASMDIR)/%.s: $(SRCDIR)/**/%.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/%.s: $(SRCDIR)/%.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(OBJDIR)/%.o: $(ASMDIR)/%.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(ASMDIR)/$(BINNAME)_hash.s: $(SRCDIR)/bench/ref/ref_$(BINNAME).c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(OBJDIR)/$(BINNAME)_hash.o: $(ASMDIR)/$(BINNAME)_hash.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

pf_arithmetic:
	cp $(SRCDIR)/field_arithmetic/pf_arithmetic_$(PRIMETYPE)_$(PRIMENAME)_$(LIMBBITS)_$(WORDSIZE)_$(METHOD).h $(SRCDIR)/field_arithmetic/field_arithmetic.h

mersenne_arithmetic:
	cp $(SRCDIR)/field_arithmetic/mersenne_arithmetic_$(PRIMETYPE)_$(PI)_$(LIMBBITS)_$(WORDSIZE)_$(METHOD).h $(SRCDIR)/field_arithmetic/field_arithmetic.h

bf_arithmetic:
	cp $(SRCDIR)/field_arithmetic/bf_arithmetic_$(FIELDSIZE)_$(LIMBBITS)_$(WORDSIZE)_$(METHOD).h $(SRCDIR)/field_arithmetic/field_arithmetic.h

build: $(DEPS) $(OBJDIR)/main.o
	$(CC) $(CCFLAGS) $(DEFS) -o bin/$(BINNAME) $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

build_ctgrind: $(DEPS) $(OBJDIR)/ctgrind_main.o
	$(CC) $(CCFLAGS) $(DEFS) -o bin/$(BINNAME)_ctgrind $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

build_bench: $(DEPS) $(OBJDIR)/bench.o
	$(CC) $(CCFLAGS) $(DEFS) -o bin/$(BINNAME)_bench $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

build_lib: $(DEPS) $(OBJDIR)/hash.o
	$(CC) -shared $(CCFLAGS) $(DEFS) -o bin/$(BINNAME).so $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

build_arith_test: $(SRCDIR)/field_arithmetic/field_arithmetic_test$(PC).c
	$(CC) $(CCFLAGS) -fpic -c $(DEFS) -o $(OBJDIR)/field_arithmetic_test.o $^ $(INCDIRS)
	$(CC) -shared -o $(BINDIR)/$(BINNAME)_arithmetic.so ${OBJDIR}/field_arithmetic_test.o $(call uniq,$(__DEPS))

build_binary_arith_test: $(SRCDIR)/field_arithmetic/bf_field_arithmetic_test.c
	$(CC) $(CCFLAGS) -fpic -c $(DEFS) -o ${OBJDIR}/bf_field_arithmetic_test.o $^ $(INCDIRS)
	$(CC) -shared -o $(BINDIR)/$(BINNAME)_arithmetic.so ${OBJDIR}/bf_field_arithmetic_test.o $(call uniq,$(__DEPS))

build_reference: $(REFDEPS) $(OBJDIR)/bench.o
	$(CC) $(CCFLAGS) $(DEFS) -o bin/$(BINNAME)_bench $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

pretty_print_intermediary:
	clang-format --style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i $(ASMDIR)/*.i

pretty_print_all_intermediary:
	clang-format --style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i $(_ASMDIR)/**/*.i

bench: build_bench
	./bin/$(BINNAME)_bench

bench_reference: build_reference
	./bin/$(BINNAME)_bench

clean:
	rm -fr $(_OBJDIR)/* $(_ASMDIR)/*

binclean:
	rm -rf $(BINDIR)

plotclean:
	rm -fr $(PLOTDIR)

resultclean:
	rm -fr $(TESTRESDIR)

deepclean: clean binclean plotclean resultclean
	rm -fr $(_OBJDIR) $(_ASMDIR)

.PRECIOUS: $(ASMDIR)/%.s

ifndef VERBOSE
.SILENT:
endif
