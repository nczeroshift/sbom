
/**
 * BXON - Binary eXchange Object Notation
 * Under MIT License
 * Copyright © Luís F. Loureiro
 * https://github.com/nczeroshift/bxon
 * Version ??.??.??
 * 2015-08-05
 */

#ifndef BXON_H_
#define BXON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Native Types
typedef enum{
    BXON_NIL        = 0,
    BXON_STRING     = 1,
    BXON_BOOLEAN    = 2,
    BXON_INT        = 3,
    BXON_LONG       = 4,
    BXON_FLOAT      = 5,
    BXON_DOUBLE     = 6,
    BXON_BYTE       = 7,
} bxon_type;

// Length Flag
typedef enum{
    BXON_LENGTH_8   = 0x00,
    BXON_LENGTH_16  = 0x10,
    BXON_LENGTH_32  = 0x20,
    BXON_LENGTH_64  = 0x30,
} bxon_length;

// Object Flag
typedef enum{
    BXON_OBJECT     = 0x00,
    BXON_ARRAY      = 0x40,
    BXON_MAP        = 0x80,
} bxon_flag;

// Error Codes
typedef enum{
    BXON_ERROR_NONE                 = 0,
    BXON_ERROR_ARRAY_OR_MAP_MISSING = 1,
} bxon_error;

// Byte Masks
#define BXON_MASK_TYPE      0x0F
#define BXON_MASK_LENGTH    0x30
#define BXON_MASK_FLAG      0xC0

struct bxon_context;

typedef uint32_t (bxon_read)    (struct bxon_context * context, int32_t size, uint8_t * d);
typedef uint32_t (bxon_write)   (struct bxon_context * context, int32_t size, uint8_t * d);
typedef uint64_t (bxon_seek)    (struct bxon_context * context, int64_t offset);
typedef uint64_t (bxon_tell)    (struct bxon_context * context);

struct bxon_context{
    bxon_read * read;
    bxon_write * write;
    bxon_seek * seek;
    bxon_tell * tell;
    uint32_t error;
    void * data;
};

struct bxon_header{
    uint8_t type;
    uint64_t length;
};

struct bxon_object{
    struct bxon_header header;
    struct bxon_context * context;
    void * data;
};

struct bxon_data_array{
    uint32_t initCapacity;
    uint32_t capacity;
    uint32_t size;
    void * objects;
};

struct bxon_data_map{
    uint32_t initCapacity;
    uint32_t capacity;
    uint32_t size;
    char ** keys;
    void ** objects;
};

void                    bxon_release    (struct bxon_object ** obj);

uint8_t                 bxon_is_map     (struct bxon_object * obj);
uint8_t                 bxon_is_array   (struct bxon_object * obj);
uint8_t                 bxon_get_type   (struct bxon_object * obj);

struct bxon_object *    bxon_new_nil    ();
struct bxon_object *    bxon_new_int    (int32_t value);
struct bxon_object *    bxon_new_long   (int64_t value);
struct bxon_object *    bxon_new_bool   (uint8_t value);
struct bxon_object *    bxon_new_float  (float value);
struct bxon_object *    bxon_new_double (double value);
struct bxon_object *    bxon_new_byte   (uint8_t byte);
struct bxon_object *    bxon_new_string (const char * string);

int32_t                 bxon_get_int    (struct bxon_object * obj);
int64_t                 bxon_get_long   (struct bxon_object * obj);
uint8_t                 bxon_get_bool   (struct bxon_object * obj);
float                   bxon_get_float  (struct bxon_object * obj);
double                  bxon_get_double (struct bxon_object * obj);
uint8_t                 bxon_get_byte   (struct bxon_object * obj);
const char *            bxon_get_string (struct bxon_object * obj);

struct bxon_object *    bxon_array_new          (uint8_t nativeType, uint32_t initCapacity);
uint32_t                bxon_array_size         (struct bxon_object * obj);
uint32_t                bxon_array_push         (struct bxon_object * obj, struct bxon_object * elem);
struct bxon_object *    bxon_array_get_object   (struct bxon_object * obj, uint32_t index);

struct bxon_object *    bxon_map_new        (uint32_t initCapacity);
uint32_t                bxon_map_size       (struct bxon_object * obj);
uint32_t                bxon_map_put        (struct bxon_object * obj, const char * key, struct bxon_object * elem);
struct bxon_object *    bxon_map_get_object (struct bxon_object * obj, const char * key);
const char *            bxon_map_get_key    (struct bxon_object * obj, uint32_t index);

uint64_t                bxon_write_object   (struct bxon_object * obj, struct bxon_context * ctx);
struct bxon_object *    bxon_read_object    (struct bxon_context * ctx);

#ifdef __cplusplus
}
#endif

#endif
