#bxon
**B**inary e**X**change **O**bject **N**otation  
*"Sean Connery binary JSON"*

##About
This project is a quick and simple specification to read and write **binary** data in a **JSON** fashion, it's based on **mp4 atom** structure and tries to offer a way to retreive data without full parsing.

##Structure
Each object contains an header and data. In the header is stored the type tag and length. In data is stored object contents, which may be native or other objects.

####Object
[ **type** (1 byte), **length** (0 to 8 bytes), **data** (N bytes) ]

##### Native
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

##### Collections
* map [ header (1 byte) | length(0-8 bytes) | data (N bytes)]
* array [ header (1 byte) | length(0-8 bytes) | data (N bytes)]

##### Arrays and Natives
Arrays can be constructed only with native types, this way it uses no extra bytes to store arrays of float, int, etc..

Type | Structure
---- | ---------
array | [ header (1 byte), length(0-8 bytes), data (N * len(int,float,double,byte,etc...)]

##Attention
This a work in progress, specification may not be final.
