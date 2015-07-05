#bxon
**B**inary e**X**change **O**bject **N**otation  
*"Sean Connery binary JSON"*

##About
This project is a quick and simple specification to read and write **binary** data in a **JSON** fashion, it's based on **mp4 atom** structure and tries to offer a way to retreive data without full parsing.

##Structure
Each object contains an header and data. In the header is stored the type tag and length. In data is stored object contents, which may be native or other objects.

####Object
[ **type** (1 byte), **length** (0 to 8 bytes), **data** (N bytes) ]

##### Header Type
Flag | Value | Group
---- | ----- | -----
BXON_NIL| 0 | Native type
BXON_STRING|1| Native type
BXON_BOOLEAN| 2| Native type
BXON_INT|3| Native type
BXON_LONG|4| Native type
BXON_FLOAT|5| Native type
BXON_DOUBLE|6| Native type
BXON_BYTE| 7| Native type
BXON_LENGTH_8|0x00| Length field size
BXON_LENGTH_16|0x10| Length field size
BXON_LENGTH_32|0x20| Length field size
BXON_LENGTH_64|0x30| Length field size
BXON_OBJECT|0x00| Object Type
BXON_ARRAY|0x40| Object Type
BXON_MAP|0x80| Object Type

##### Header Length
This is where the object length in bytes is set, this field can vary from 0 to 8 bytes depending on the needed integer 
value.

##### Native Type Structure
Type | Structure
---- | ---------
nil | [ header (1 byte) ]
boolean | [ header (1 byte), data (1 byte)  ]
int  | [ header (1 byte), data (4 bytes) ]
long | [ header (1 byte), data (8 bytes) ]
byte | [ header (1 byte), data (1 byte)  ]
float | [ header (1 byte), data (4 bytes) ]
double | [ header (1 byte), data (8 bytes) ]
string | [ header (1 byte), length(0-8 bytes), data (N bytes)]

##### Collections Type Structure
Type | Structure
---- | ---------
map | [ header (1 byte), length(0-8 bytes), data (N * [Entry Key Object, Entry Value Object])]
array | [ header (1 byte), length(0-8 bytes), data (N * [ Entry Value Object])]

##### Arrays and Natives
Arrays can be constructed only with native types, this way it uses no extra bytes to store arrays of float, int, etc..

Type | Structure
---- | ---------
array | [ header (1 byte), length(0-8 bytes), data (N * len(int,float,double,byte,etc...))]

##Attention
This a work in progress, specification may not be final.
