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
INCDIRS=-Iinclude -I/opt/local/include -I/local/openssl3/include -Ihacl_include -Ihaberdashery_include -I/usr/include
LIBDIRS= -L/opt/local/lib -Llib/ -L/usr/lib #-L/user/local/ssl/lib64

_DEFS=-DKEYINCLUDE_H=\"$(KEYINCLUDE).h\" -DKEYTRANSFORM=$(KEYTRANSFORM)\
     -DMSGINCLUDE_H=\"$(MSGINCLUDE).h\" -DMSGTRANSFORM=$(MSGTRANSFORM)\
	 -DFIELDELEMINCLUDE_H=\"$(FIELDELEMINCLUDE).h\" -DFIELDELEMTRANSFORM=$(FIELDELEMTRANSFORM)\
	 -DOUTERPOLY_H=\"$(OUTERPOLY_H)\" -DOUTERPOLY=$(OUTERPOLY) $(MDEFS) -DFOLDER=\"$(BENCHDIR)\" -DNAME=\"$(BENCHRESNAME)\"
__DEPS = $(OBJDIR)/$(KEYINCLUDE).o $(OBJDIR)/$(MSGINCLUDE).o $(OBJDIR)/$(FIELDELEMINCLUDE).o
_DEPS = $(__DEPS) $(OBJDIR)/hash.o $(OBJDIR)/key_expansion.o $(OBJDIR)/randombytes.o $(OBJDIR)/$(OUTERPOLY).o
REFDEPS = $(OBJDIR)/$(BINNAME)_hash.o $(OBJDIR)/randombytes.o

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

override _DEPS := $(_DEPS) $(ADDDEPS)
override CCFLAGS := $(CCFLAGS) -std=c11 -Wall -fstack-protector -Werror -fpic -save-temps=obj -ggdb
override LDFLAGS := $(LDFLAGS) # -lm

UNAME_S := $(shell uname -s)
CC_V := $(shell $(CC) --version)
ifeq ($(UNAME_S),Darwin)
	override CCFLAGS := $(CCFLAGS) -Wno-error=unused-command-line-argument -Wno-unused-command-line-argument
else
ifeq ($(findstring clang,$(CC_V)),clang)
	override CCFLAGS := $(CCFLAGS) -Wno-error=unused-command-line-argument -Wno-unused-command-line-argument
endif
endif

ifeq ($(USE_D2LHASH),1)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/d2LHash1271.o $(OBJDIR)/d2LHash1271_maax.o $(OBJDIR)/d2LHash1271_const.o $(OBJDIR)/d2LHash1271_keypowers.o
	override INCDIRS := $(INCDIRS) -Iref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/include/
	override CCFLAGS := $(CCFLAGS) -Wno-unused-variable -m64 -O3 -funroll-loops -fomit-frame-pointer
endif

ifeq ($(USE_D2LHASH),2)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/d2LHash1305.o $(OBJDIR)/d2LHash1305_maax.o $(OBJDIR)/d2LHash1305_const.o $(OBJDIR)/d2LHash1305_keypowers.o
	override INCDIRS := $(INCDIRS) -Iref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/include/
	override CCFLAGS := $(CCFLAGS) -Wno-unused-variable -m64 -O3 -funroll-loops -fomit-frame-pointer
endif

ifeq ($(USE_HASH2L),1)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/hash_128_karatmult3.o
	override CCFLAGS := $(CCFLAGS) -Wno-unused-result -mpclmul -mavx2
endif
ifeq ($(USE_HASH2L),2)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/hash_128_sbmult3.o
	override CCFLAGS := $(CCFLAGS) -Wno-unused-result -mpclmul -mavx2
endif
ifeq ($(USE_HASH2L),3)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/hash_256_karatmult1.o
	override CCFLAGS := $(CCFLAGS) -Wno-unused-result -mpclmul -mavx2 -Wno-incompatible-pointer-types
endif

ifeq ($(USE_POLYHASH),1)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/poly1305.o $(OBJDIR)/poly1305_const.o $(OBJDIR)/poly1305_keypowers.o $(OBJDIR)/poly1305_maax.o
	override INCDIRS := $(INCDIRS) -Iref/poly1305/$(POLYHASHG)/include/
	override CCFLAGS := $(CCFLAGS) -Wno-unused-variable -m64 -O3 -funroll-loops -fomit-frame-pointer
endif
ifeq ($(USE_POLYHASH),2)
	override REFDEPS := $(REFDEPS) $(OBJDIR)/poly1305.o $(OBJDIR)/poly1305_const.o $(OBJDIR)/poly1305_maax.o
	override INCDIRS := $(INCDIRS) -Iref/poly1305/$(POLYHASHG)/include/
	override CCFLAGS := $(CCFLAGS) -Wno-unused-variable -m64 -O3 -funroll-loops -fomit-frame-pointer
endif

ifdef USE_HABERDASHERY
	override REFDEPS := $(REFDEPS) $(OBJDIR)/aes256gcm_broadwell.o $(OBJDIR)/aes256gcm_haswell.o $(OBJDIR)/aes256gcm_skylake.o $(OBJDIR)/aes256gcm_skylakex.o $(OBJDIR)/aes256gcm_tigerlake.o
endif

ifdef USE_OPEN_SSL
	override _DEFS := $(_DEFS) -DUSE_OPEN_SSL
	override LDFLAGS := $(LDFLAGS) -lssl -lcrypto
else
ifdef USE_HACL
	override _DEFS := $(_DEFS) -D_DEFAULT_SOURCE
	override LDFLAGS := $(LDFLAGS) -l:libevercrypt.a
	ifeq ($(UNAME_S),Darwin)
		override CCFLAGS := $(CCFLAGS) -Wno-incompatible-pointer-types-discards-qualifiers
	else
	ifeq ($(findstring clang,$(CC_V)),clang)
		override CCFLAGS := $(CCFLAGS) -Wno-incompatible-pointer-types-discards-qualifiers
	else
		override CCFLAGS := $(CCFLAGS) -Wno-discarded-qualifiers
	endif
	endif
endif
	override LDFLAGS := $(LDFLAGS) -lsodium
endif
ifdef USE_CTGRIND
	override _DEFS := $(_DEFS) -DUSE_CTGRIND
	override LDFLAGS := $(LDFLAGS) -lctgrind
endif

ifdef INNERPOLY
# DEPS = $(_DEPS) $(OBJDIR)/$(INNERPOLY).o
DEFS = $(_DEFS) -DINNERPOLY_H=\"$(INNERPOLY_H)\" -DINNERPOLY=$(INNERPOLY)
else
DEFS = $(_DEFS)
endif
DEPS = $(_DEPS)

dir:
	mkdir -p $(OBJDIR) $(BINDIR) $(ASMDIR) $(BENCHDIR) $(TESTRESDIR) $(PLOTDIR)

$(ASMDIR)/d2LHash1271.s: ref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1271.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

ifeq ($(UNAME_S),Darwin)
$(OBJDIR)/d2LHash1271_keypowers.o: ref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1271_keypowers.S
	$(CC) $(DEFS) $(CCFLAGS) -Dd2LHash1271keypowers=_d2LHash1271keypowers -o $@ -c $< $(INCDIRS)

$(OBJDIR)/d2LHash1271_maax.o: ref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1271_maax.S
	$(CC) $(DEFS) $(CCFLAGS) -Dd2LHash1271maax=_d2LHash1271maax -o $@ -c $< $(INCDIRS)

$(OBJDIR)/d2LHash1271_const.o: ref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1271_const.S
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)
else
$(OBJDIR)/d2LHash1271_%.o: ref/d2LHash/d2LHash1271/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1271_%.S
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)
endif

$(OBJDIR)/d2LHash1271.o: $(ASMDIR)/d2LHash1271.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)


$(ASMDIR)/d2LHash1305.s: ref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1305.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/poly1305.s: ref/poly1305/$(POLYHASHG)/source/poly1305.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

ifeq ($(UNAME_S),Darwin)
$(OBJDIR)/d2LHash1305_keypowers.o: ref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1305_keypowers.S
	$(CC) $(DEFS) $(CCFLAGS) -Dd2LHash1305keypowers=_d2LHash1305keypowers -o $@ -c $< $(INCDIRS)

$(OBJDIR)/d2LHash1305_maax.o: ref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1305_maax.S
	$(CC) $(DEFS) $(CCFLAGS) -Dd2LHash1305maax=_d2LHash1305maax -o $@ -c $< $(INCDIRS)

$(OBJDIR)/d2LHash1305_const.o: ref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1305_const.S
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)
else
$(OBJDIR)/d2LHash1305_%.o: ref/d2LHash/d2LHash1305/$(D2LHASHD)/$(D2LHASHG)/source/d2LHash1305_%.S
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(OBJDIR)/poly1305_%.o: ref/poly1305/$(POLYHASHG)/source/poly1305_%.S
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)
endif

$(OBJDIR)/d2LHash1305.o: $(ASMDIR)/d2LHash1305.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(OBJDIR)/poly1305.o: $(ASMDIR)/poly1305.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(ASMDIR)/hash_128_karatmult3.s: ref/**/hash_128_karatmult3/hash_128_karatmult3.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/hash_128_sbmult3.s: ref/**/hash_128_sbmult3/hash_128_sbmult3.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/hash_256_karatmult1.s: ref/**/hash_256_karatmult1/hash_256_karatmult1.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/%.s: $(SRCDIR)/**/%.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(ASMDIR)/%.s: $(SRCDIR)/%.c
	$(CC) -S $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS) $(LDFLAGS)

$(OBJDIR)/aes256gcm_%.o: haberdashery_include/aes256gcm_%.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(OBJDIR)/hash_128_karatmult3.o: $(ASMDIR)/hash_128_karatmult3.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(OBJDIR)/hash_128_sbmult3.o: $(ASMDIR)/hash_128_sbmult3.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

$(OBJDIR)/hash_256_karatmult1.o: $(ASMDIR)/hash_256_karatmult1.s
	$(CC) $(DEFS) $(CCFLAGS) -o $@ -c $< $(INCDIRS)

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

build_arith_test: $(SRCDIR)/field_arithmetic/field_arithmetic_test$(PC).c
	$(CC) $(CCFLAGS) -fpic -c $(DEFS) -o $(OBJDIR)/field_arithmetic_test.o $^ $(INCDIRS)
	$(CC) -shared -o $(BINDIR)/$(BINNAME)_arithmetic.so ${OBJDIR}/field_arithmetic_test.o $(call uniq,$(__DEPS))

build_binary_arith_test: $(SRCDIR)/field_arithmetic/bf_field_arithmetic_test.c
	$(CC) $(CCFLAGS) -fpic -c $(DEFS) -o ${OBJDIR}/bf_field_arithmetic_test.o $^ $(INCDIRS)
	$(CC) -shared -o $(BINDIR)/$(BINNAME)_arithmetic.so ${OBJDIR}/bf_field_arithmetic_test.o $(call uniq,$(__DEPS))

build_lib: $(DEPS) $(OBJDIR)/hash.o
	$(CC) -shared $(CCFLAGS) $(DEFS) -o bin/$(BINNAME).so $^ $(INCDIRS) $(LIBDIRS) $(LDFLAGS)

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
