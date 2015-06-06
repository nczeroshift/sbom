
#ifndef BXON_H_
#define BXON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
typedef enum{
    BXON_OBJECT = 0,
    
    // native types
	BXON_STRING = 1,
	BXON_BOOLEAN = 2,
	BXON_INT = 3,
	BXON_LONG = 4, 
	BXON_FLOAT = 4,
	BXON_DOUBLE = 5,
    BXON_BYTE = 6,
    
    // available types
	BXON_MAP = 32,
	BXON_ARRAY = 64,
	BXON_EXTENDED = 128,
} bxon_types;

typedef enum{
    BXON_ERROR_NONE = 0,
    BXON_ERROR_ARRAY_OR_MAP_MISSING = 1
}bxon_error;

#define BXON_MASK_TYPE 0x1F
#define BXON_MASK_FLAG 0xE0

struct bxon_header{
	uint32_t type;
	uint64_t length;
};

struct bxon_object{
    struct bxon_header header;
    void * data;
};

struct bxon_data_array{
    uint32_t capacity;
    uint32_t size;
    void * objects;
    int64_t * offset;
};
    
struct bxon_data_map{
    uint32_t capacity;
    uint32_t size;
    char ** keys;
    void ** objects;
    int64_t * offset;
};
    

struct bxon_object *    bxon_new        ();
void                    bxon_release    (struct bxon_object ** obj);

struct bxon_object *    bxon_new_array      (uint8_t nativeType, int initCapacity);
void                    bxon_array_release  (struct bxon_object * obj);

struct bxon_object *    bxon_new_map     (int initCapacity);
void                    bxon_map_release (struct bxon_object * obj);

struct bxon_object *    bxon_new_int    (int32_t value);
struct bxon_object *    bxon_new_long   (int64_t value);
struct bxon_object *    bxon_new_bool   (uint8_t value);
struct bxon_object *    bxon_new_float  (float value);
struct bxon_object *    bxon_new_double (double value);
struct bxon_object *    bxon_new_byte   (uint8_t byte);
struct bxon_object *    bxon_new_string (char * string);

uint8_t                 bxon_is_map     (struct bxon_object * obj);
uint8_t                 bxon_is_array   (struct bxon_object * obj);
uint8_t                 bxon_get_type   (struct bxon_object * obj);

int32_t                 bxon_get_int    (struct bxon_object * obj);
int64_t                 bxon_get_long   (struct bxon_object * obj);
uint8_t                 bxon_get_bool   (struct bxon_object * obj);
float                   bxon_get_float  (struct bxon_object * obj);
double                  bxon_get_double (struct bxon_object * obj);
uint8_t                 bxon_get_byte   (struct bxon_object * obj);
int32_t                 bxon_get_string (struct bxon_object * obj, char ** string);
    
#ifdef __cplusplus
}
#endif

#endif
