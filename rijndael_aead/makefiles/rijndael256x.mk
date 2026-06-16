rijndael256x2x_no_hash: $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x05F1 $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x3x_no_hash: $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x05F2 $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x4x_no_hash: $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x05F3 $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x6x_no_hash: $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x05F4 $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x8x_no_hash: $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x05F5 $(SRC)/rijndael256_no_hash.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)


rijndael256x_poly128x2: $(SRC)/rijndael256x_poly128.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0531 $(SRC)/rijndael256x_poly128.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_poly128x4: $(SRC)/rijndael256x_poly128.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0532 $(SRC)/rijndael256x_poly128.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)

rijndael256x_poly256x2: $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0511 $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_poly256x3: $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0512 $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_poly256x4: $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0513 $(SRC)/rijndael256x_poly256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)

rijndael256x_mhp_nmh128x4: $(SRC)/rijndael256x_mhp_nmh128.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0521 $(SRC)/rijndael256x_mhp_nmh128.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_mhp_nmh128x6: $(SRC)/rijndael256x_mhp_nmh128.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0520 $(SRC)/rijndael256x_mhp_nmh128.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)

rijndael256x_mhp_nmh256x2: $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0501 $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_mhp_nmh256x4: $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0502 $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)
rijndael256x_mhp_nmh256x6: $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c $(SRC)/test_aead.h $(INC)/*
	$(CC) -I$(INC) $(CCOPTS) $(BUILDOPT) -DNAME=\"$@\" -DUSED_AEAD=0x0500 $(SRC)/rijndael256x_mhp_nmh256.c $(SRC)/$(MODE)_aead.c -o $(BIN)/$@_$(MODE)

all_rijndael256x: \
	rijndael256x2x_no_hash \
	rijndael256x3x_no_hash \
	rijndael256x4x_no_hash \
	rijndael256x6x_no_hash \
	rijndael256x8x_no_hash \
	rijndael256x_poly128x2 \
	rijndael256x_poly128x4 \
	rijndael256x_poly256x2 \
	rijndael256x_poly256x3 \
	rijndael256x_poly256x4 \
	rijndael256x_mhp_nmh128x4 \
	rijndael256x_mhp_nmh128x6 \
	rijndael256x_mhp_nmh256x2 \
	rijndael256x_mhp_nmh256x4 \
	rijndael256x_mhp_nmh256x6 

