# BBF file format

Specification of the binary bitmap file (BBF) format. It is a simple raw data file format without compression for images. It was created by Benjamin Buch.

## BBF signature

The first four bytes of a BBF datastream always contain the following (hexadecimal) values:

```text
0x62 0x62 0x66 0x21 ("BBF!" as ASCII)
```

## File format version

The fifth byte is always the version. The version is a unsigned integer with 8 bit. The first version is `0x00`.

## BBF file format version `0x00`

A version `0x00` BBF-File is devided into a header and a data section.

### Data types

The format stores a two-dimensional rectangular array of pixels. Each pixel can consist of up to 255 channels of the same data type. Permitted data types are:

* boolean (8 bit)
* un/signed integer 8 bit
* un/signed integer 16 bit
* un/signed integer 32 bit
* un/signed integer 64 bit
* floating point IEEE 754 32 bit
* floating point IEEE 754 64 bit

### Header

The header is encoded in **big endian**:

* unsigned integer 8 bit: size of channel type in byte (e.g. 2 for 16 bit integer)
* unsigned integer 8 bit: count of channels per pixel (e.g. 3 for rgb pixels)
* unsigned integer 8 bit: flags (see below)
* unsigned integer 64 bit: width in pixels
* unsigned integer 64 bit: height in pixels

The flags encode the data in the lower half byte and the endianness in the upper half.

* `0x?0`: unsigned integer
* `0x?1`: signed integer
* `0x?2`: floating point
* `0x?3`: boolean
* `0x0?`: big endian
* `0x1?`: little endian

### Data

The pixels are stored in line wise order. Each pixel has a size of channel type size times channel count. The values are are encoded with the endianness defined in the flags.

