# (New) Json Input

Each configuration is specified as json object.
As a consequence of that the order of fields does not matter.
Furthermore the parsing ignores any custom additional fields,
but we do not recommend creating custom fields in your config,
as new fields might be added when new features are added to the framework.
A configuration file contains a list of these configuration objects.

If it contains more than a single configuration, then the configuration file has the following structure (optional fields are indicated with a `?` after their type):

```js
{
    "name": str?
    "description": str?
    "configurations": [Config]
}
```

## Configs

Every config has a name and a optional skip and description fields.
If skip is true the framework will skip that configuration.
A config object is either a ReferenceConfig or a NewHashConfig.

### Reference Config

A reference config has the following format:
```js
{
    "name": str
    "ref": true
    "lib": str
    "mac": str
    "implementation": str?
    "skip": bool?
    "description": str?
}
```

the following combinations of values are currently supported:
|lib     |mac     |implementation|Resulting Reference Implementation  |
|--------|--------|--------------|------------------------------------|
|sodium  |poly1305| -            |Libsodium's Poly1305                |
|openssl |poly1305| -            |Openssl's Poly1305                  |
|openssl |gmac    | -            |Openssl's GMAC                      |
|haclstar|poly1305| -            |HACL* Poly1305 (Intger units)       |
|haclstar|poly1305|128           |HACL* Poly1305 (SSE, 128bit vectors)|
|haclstar|poly1305|256           |HACL* Poly1305 (AVX, 255bit vectors)|

### New Hash Config
A config for a new Hash has the following format:
A reference config has the following format:

```js
{
    "name": str
    "ref": false
    "keysize": int
    "blocksize": int
    "tagsize": int
    "field": FieldSpec
    "wordsize": 32 | 64
    "limbs": [int]
    "multiplication": "schoolbook"
    "key_transform": {
        "id": int
        "options": {
            "mask": HexInt
        }
    }
    "msg_transform": {
        "id": int
        "options":  {
            "byte": HexInt
            "mask": list[HexInt]
            "encodeLSB": bool
        }
    }
    "field_transform": {
        "id": int
    }
    "polynomial": PolynomialSpec
    "keygenerator": {
        "required": bool
        "number_of_bytes": int?
    }
    "skip": bool?
    "description": str?
}
```

where `HexInt` is either an integer or a string representing an integer in hex.
And a `PolynomialSpec` is the following recursive json object.

```js
{
    "name": str
    "parameters": [int]
    "inner_polynomial": PolynomialSpec?
}
```

A `FieldSpec` is one of the following:

```js
{
    field_type: "crandallprime"
    pi: int
    delta: int
}

{
    field_type: "mersenneprime"
    pi: int
}

{
    field_type: "binary"
    size: int
}
```
