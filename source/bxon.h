
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

struct bxon_context;
struct bxon_header;

typedef int32_t (bxon_proto_read) (struct bxon_context * context, int32_t size, uint8_t * d);
typedef int32_t (bxon_proto_write) (struct bxon_context * context, int32_t size, uint8_t * d);
typedef int64_t (bxon_proto_seek) (struct bxon_context * context, int64_t offset);

typedef int8_t (bxon_proto_parse_callback) (struct bxon_context * context,struct bxon_header * header);


    
struct bxon_context{
	bxon_proto_read * read;
	bxon_proto_write * write;
    bxon_proto_seek * seek;
    uint32_t error;
	void * priv;
};

struct bxon_header{
	//char * name;
	uint32_t type;
	uint64_t length;
};
    
struct bxon_data_vector{
    uint32_t capacity;
    uint32_t size;
    void ** objects;
};
    
struct bxon_data_map{
    uint32_t capacity;
    uint32_t size;
    char ** keys;
    void ** objects;
};
    
struct bxon_object{
    struct bxon_header header;
    void * data;
};
    
void                    bxon_header_init        (struct bxon_header * header);
void                    bxon_header_release     (struct bxon_header * header);
    
void                    bxon_object_init        (struct bxon_object * obj);
void                    bxon_object_release     (struct bxon_object * obj);
    
void                    bxon_map_init           (struct bxon_object * map);
void                    bxon_map_release        (struct bxon_object * map);
uint8_t                 bxon_map_add_object     (struct bxon_object * map, struct bxon_object * obj, char * key);
uint8_t                 bxon_map_contains_key   (struct bxon_object * map, char * key);
struct bxon_object *    bxon_map_remove_key     (struct bxon_object * map, char * key);
uint32_t                bxon_map_size           (struct bxon_object * map, char * key);
uint8_t                 bxon_map_get_key        (struct bxon_object * map, uint32_t index, char ** key);
uint8_t                 bxon_map_get_object     (struct bxon_object * map, char * key);
    
void                    bxon_vector_init        (struct bxon_object * vector, uint8_t type);
void                    bxon_vector_release     (struct bxon_object * vector);
uint8_t                 bxon_vector_get_type    (struct bxon_object * vector);
void                    bxon_vector_set_type    (struct bxon_object * vector, uint8_t type);
uint32_t                bxon_vector_size        (struct bxon_object * vector);
struct bxon_object *    bxon_vector_get_object  (struct bxon_object * vector, uint32_t index);
uint8_t                 bxon_vector_push_object (struct bxon_object * vector, struct bxon_object * obj);
uint8_t                 bxon_vector_put_object  (struct bxon_object * vector, uint32_t index, struct bxon_object * obj);
    
uint8_t                 bxon_vector_put_int     (struct bxon_object * vector, uint32_t index, int32_t value);
uint8_t                 bxon_vector_put_float   (struct bxon_object * vector, uint32_t index, float value);
uint8_t                 bxon_vector_put_double  (struct bxon_object * vector, uint32_t index, double value);
uint8_t                 bxon_vector_put_byte    (struct bxon_object * vector, uint32_t index, double value);
    
void                    bxon_object_make_int    (struct bxon_object * obj, int32_t value);
void                    bxon_object_make_long   (struct bxon_object * obj, int64_t value);
void                    bxon_object_make_bool   (struct bxon_object * obj, uint8_t value);
void                    bxon_object_make_float  (struct bxon_object * obj, float value);
void                    bxon_object_make_double (struct bxon_object * obj, double value);
void                    bxon_object_make_byte   (struct bxon_object * obj, uint8_t byte);
void                    bxon_object_make_string (struct bxon_object * obj, char * string);

uint8_t                 bxon_object_get_type   (struct bxon_object * obj);
int32_t                 bxon_object_get_int    (struct bxon_object * obj);
int64_t                 bxon_object_get_long   (struct bxon_object * obj);
uint8_t                 bxon_object_get_bool   (struct bxon_object * obj);
float                   bxon_object_get_float  (struct bxon_object * obj);
double                  bxon_object_get_double (struct bxon_object * obj);
uint8_t                 bxon_object_get_byte   (struct bxon_object * obj);
int32_t                 bxon_object_get_string (struct bxon_object * obj, char ** string);
    


uint32_t bxon_read_string(struct bxon_context * context, struct bxon_header * header, char ** string);
uint8_t bxon_read_int(struct bxon_context * context, struct bxon_header * header, int * value);
uint8_t bxon_read_long(struct bxon_context * context, struct bxon_header * header, int64_t * value);
uint8_t bxon_read_float(struct bxon_context * context, struct bxon_header * header, float * value);
uint8_t bxon_read_double(struct bxon_context * context, struct bxon_header * header, double * value);
uint8_t bxon_read_bool(struct bxon_context * context, struct bxon_header * header, uint8_t * value);

uint8_t bxon_header_read(struct bxon_context * context, struct bxon_header * header);

uint8_t bxon_parse_document(struct bxon_context * context, bxon_proto_parse_callback callback);

void bxon_write_header(struct bxon_context * context, struct bxon_header * header);

#ifdef __cplusplus
}
#endif

#endif