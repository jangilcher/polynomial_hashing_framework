# Old Grammar
Each line in a configuration defines one configuration.
Lines starting with `#` are ignored, i.e. can be used for comments.

A single configuration line  for a new Hash is specified using the following regular expressions:
```text
Blocksize := integer_const
Keysize := integer_const
Outputsize := integer_const
Interface := Blocksize Keysize Outputsize

PrimeType := integer_const
PrimeFieldSpec := pf PrimeType integer_const+
FieldSize := 64 | 128 | 192 | 256
BinaryFieldSpec := bf FieldSize
FieldSpec := PrimeFieldSpec | BinaryFieldSpec

Arch := x86
Wordsize := 32 | 64
NumLimbs := integer_const
Limbsize := integer_const
MultiplicationMethod := schoolbook
FielArithSpec := FieldSpec Arch Wordsize NumLimbs Limbsize+ MultiplicationMethod

Mask = hex_integer_const
KeyTransform := integer_const (clamp Mask)?
MessageTransform := integer_const (low? byte_const)? Mask*
FieldToBitTransform := integer_const
TransformSpecs := KeyTransform MessageTransform FieldToBitTransform

NestLevel := 0 | 1
PolynomialName := string_constant
PolynomialSpec := NestLevel (string_constant integer_constant*)+

KeygeneratorSpec := kg integer_const

ConfigName := string_constant

ConfigLine := KeygeneratorSpec? Interface FieldArithSpec TransformSpecs PolynomialSpec ConfigName
```

A configuration for a reference configuration has the following format:

```text
reference_implementation name
```

where name is some string that is used for the legend during plotting and reference_implementation is chosen as follows:

|reference_implementation  |Resulting Reference Implementation  |
|--------------------------|--------------------                |
|`refsodium_poly1305`      |Libsodium's Poly1305                |
|`refopenssl_poly1305`     |Openssl's Poly1305                 |
|`refopenssl_gmac`         |Openssl's GMAC                     |
|`refhaclstar_poly1305`    |HACL* Poly1305 (Intger units)       |
|`refhaclstar_poly1305_128`|HACL* Poly1305 (SSE, 128bit vectors)|
|`refhaclstar_poly1305_256`|HACL* Poly1305 (AVX, 255bit vectors)|
