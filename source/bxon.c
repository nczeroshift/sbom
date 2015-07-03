
#include "bxon.h"

#include <stdlib.h>
#include <string.h>

#define BXON_INIT_CAPACITY 10

#define BXON_READ(ctx,size,data)    (ctx->read) (ctx,size,(uint8_t*)data)
#define BXON_WRITE(ctx,size,data)   (ctx->write)(ctx,size,(uint8_t*)data)
#define BXON_TELL(ctx)              (ctx->tell) (ctx)
#define BXON_SEEK(ctx,pos)          (ctx->seek) (ctx,pos)

struct bxon_object *    bxon_new            ();
void                    bxon_map_release    (struct bxon_object * obj);
void                    bxon_array_release  (struct bxon_object * obj);

uint32_t                bxon_native_size    (uint8_t type);
struct bxon_object *    bxon_read_native    (struct bxon_context * ctx, uint8_t type);

struct bxon_object * bxon_new(){
    struct bxon_object * obj = calloc(1,sizeof(struct bxon_object));
    obj->header.type = BXON_OBJECT;
    obj->header.length = 0;
    obj->context = NULL;
    obj->data = NULL;
    return obj;
}

uint64_t bxon_read_length(struct bxon_context * ctx, uint8_t properties){
    uint8_t lFlag = properties & BXON_MASK_LENGTH;
    if(lFlag == BXON_LENGTH_8){
        uint8_t len;
        BXON_READ(ctx,sizeof(uint8_t),&len);
        return len;
    }else if(lFlag == BXON_LENGTH_16){
        uint16_t len;
        BXON_READ(ctx,sizeof(uint16_t),&len);
        return len;
    }else if(lFlag == BXON_LENGTH_32){
        uint32_t len;
        BXON_READ(ctx,sizeof(uint32_t),&len);
        return len;
    }else if(lFlag == BXON_LENGTH_64){
        uint64_t len;
        BXON_READ(ctx,sizeof(uint64_t),&len);
        return len;
    }
    return 0;
}

void bxon_write_length(struct bxon_context * ctx, uint8_t properties, uint64_t length){
    uint8_t lFlag = properties & BXON_MASK_LENGTH;
    if(lFlag == BXON_LENGTH_8){
        uint8_t len = (uint8_t)length;
        BXON_WRITE(ctx,sizeof(uint8_t),&len);
    }else if(lFlag == BXON_LENGTH_16){
        uint16_t len = (uint16_t)length;
        BXON_WRITE(ctx,sizeof(uint16_t),&len);
    }else if(lFlag == BXON_LENGTH_32){
        uint32_t len = (uint32_t)length;
        BXON_WRITE(ctx,sizeof(uint32_t),&len);
    }else if(lFlag == BXON_LENGTH_64){
        BXON_WRITE(ctx,sizeof(uint64_t),&length);
    }
}

uint8_t bxon_length_bytes(uint8_t properties){
    uint8_t lFlag = properties & BXON_MASK_LENGTH;
    if(lFlag == BXON_LENGTH_8){
        return sizeof(uint8_t);
    } else if(lFlag == BXON_LENGTH_16){
        return sizeof(uint16_t);
    } else if(lFlag == BXON_LENGTH_32){
        return sizeof(uint32_t);
    } else if(lFlag == BXON_LENGTH_64){
        return sizeof(uint64_t);
    }
    return 0;
}

uint8_t bxon_length_flag_for_size(uint64_t size){
    if(size > UINT8_MAX){
        if(size > UINT16_MAX){
            if(size > UINT32_MAX){
                return BXON_LENGTH_64;
            }else
                return BXON_LENGTH_32;
        }
        else
            return BXON_LENGTH_16;
    }
    else
        return BXON_LENGTH_8;
}

void bxon_release(struct bxon_object ** obj){
    if(bxon_is_map(*obj))
        bxon_map_release(*obj);
    else if(bxon_is_array(*obj))
        bxon_array_release(*obj);
    else
        free((*obj)->data);
    free(*obj);
    *obj = NULL;
}

struct bxon_object * bxon_new_nil(){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_OBJECT;
    obj->header.length = 0;
    return obj;
}

struct bxon_object * bxon_new_int(int32_t value){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_INT;
    obj->header.length = sizeof(int32_t);
    obj->data = malloc(sizeof(int32_t));
    *((int*)obj->data) = value;
    return obj;
}

struct bxon_object * bxon_new_long(int64_t value){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_LONG;
    obj->header.length = sizeof(int64_t);
    obj->data = malloc(sizeof(int64_t));
    *((int64_t*)obj->data) = value;
    return obj;
}

struct bxon_object * bxon_new_bool(uint8_t value){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_BOOLEAN;
    obj->header.length = sizeof(uint8_t);
    obj->data = malloc(sizeof(uint8_t));
    *((uint8_t*)obj->data) = value;
    return obj;
}

struct bxon_object * bxon_new_float(float value){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_FLOAT;
    obj->header.length = sizeof(float);
    obj->data = malloc(sizeof(float));
    *((float*)obj->data) = value;
    return obj;
}

struct bxon_object * bxon_new_double(double value){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_DOUBLE;
    obj->header.length = sizeof(double);
    obj->data = malloc(sizeof(double));
    *((double*)obj->data) = value;
    return obj;
}

struct bxon_object * bxon_new_byte(uint8_t byte){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_BYTE;
    obj->header.length = sizeof(uint8_t);
    obj->data = malloc(sizeof(uint8_t));
    *((uint8_t*)obj->data) = byte;
    return obj;
}

struct bxon_object * bxon_new_string(const char * string){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_STRING;
    obj->header.length = strlen(string);
    obj->data = calloc(1,obj->header.length);
    memcpy(obj->data,string,obj->header.length);
    return obj;
}

uint8_t bxon_is_map(struct bxon_object * obj){
    return ((obj->header.type & BXON_MASK_FLAG) & BXON_MAP) == BXON_MAP;
}

uint8_t bxon_is_array(struct bxon_object * obj){
    return ((obj->header.type & BXON_MASK_FLAG) & BXON_ARRAY) == BXON_ARRAY;
}

uint8_t bxon_get_type(struct bxon_object * obj){
    return obj->header.type & BXON_MASK_TYPE;
}

int32_t bxon_get_int(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE )!= BXON_INT ||
            obj->header.length != sizeof(int32_t))
        return 0;
    return *((int32_t*)obj->data);
}

int64_t bxon_get_long(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_LONG ||
            obj->header.length != sizeof(int64_t))
        return 0;
    return *((int64_t*)obj->data);
}

uint8_t bxon_get_bool(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_BOOLEAN ||
            obj->header.length != sizeof(uint8_t))
        return 0;
    return *((uint8_t*)obj->data) & 0x01;
}

float bxon_get_float(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_FLOAT ||
            obj->header.length != sizeof(float))
        return 0;
    return *((float*)obj->data);
}

double bxon_get_double(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_DOUBLE ||
            obj->header.length != sizeof(double))
        return 0;
    return *((double*)obj->data);
}

uint8_t bxon_get_byte(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_BYTE ||
            obj->header.length != sizeof(uint8_t))
        return 0;
    return *((uint8_t*)obj->data);
}

const char * bxon_get_string(struct bxon_object * obj){
    if((obj->header.type & BXON_MASK_TYPE) != BXON_STRING)
        return NULL;
    return (char*)obj->data;
}

struct bxon_object * bxon_array_new(uint8_t nativeType, uint32_t initCapacity){
    struct bxon_object * obj;
    struct bxon_data_array * array;
    obj = bxon_new();
    obj->header.type = BXON_ARRAY | BXON_LENGTH_64;
    array = (struct bxon_data_array*)calloc(sizeof(struct bxon_data_array),1);
    obj->data = array;
    if(nativeType == BXON_OBJECT || nativeType == BXON_STRING)
        array->objects = (struct bxon_data_array **)calloc(sizeof(struct bxon_data_array*),initCapacity);
    else{
        int tSize = bxon_native_size(nativeType);
        array->objects = calloc(initCapacity,tSize);
        obj->header.type |= nativeType;
    }
    array->initCapacity = array->capacity = initCapacity;
    array->size = 0;
    return obj;
}

uint32_t bxon_array_size(struct bxon_object * obj){
    struct bxon_data_array * array = obj->data;
    return array->size;
}

uint32_t bxon_array_push(struct bxon_object * obj, struct bxon_object * elem){
    struct bxon_data_array * array = obj->data;
    if(obj->header.type == BXON_OBJECT){
        if(array->objects == NULL)
            array->objects = calloc(array->initCapacity,sizeof(struct bxon_object *));

        if(array->size == array->capacity){
            uint32_t tmpCapacity = array->size + array->initCapacity;
            array->capacity = tmpCapacity;
            array->objects = realloc(array->objects, sizeof(struct bxon_object *) * tmpCapacity);
        }

        ((struct bxon_object **)array->objects)[array->size] = elem;
        array->size++;
    }else{
        uint64_t writePos;
        uint32_t natSize = bxon_native_size(elem->header.type & BXON_MASK_TYPE);

        if(!natSize)
            return 0;

        if(array->objects == NULL)
            array->objects = calloc(array->initCapacity,natSize);

        if(array->size == array->capacity){
            uint32_t tmpCapacity = array->size + array->initCapacity;
            array->capacity = tmpCapacity;
            array->objects = realloc(array->objects, natSize * tmpCapacity);
        }

        writePos = array->size * natSize;
        memcpy((uint8_t*)array->objects+writePos,elem->data,natSize);
        array->size++;
    }
    return array->size;
}

struct bxon_object * bxon_array_get_object(struct bxon_object * obj, uint32_t index){
    struct bxon_data_array * array = obj->data;
    if(index >= array->size)
        return NULL;

    if((obj->header.type & BXON_MASK_TYPE) == BXON_NIL){
        return ((struct bxon_object **)array->objects)[index];
    }else{
        struct bxon_object * ret;
        uint32_t natSize = bxon_native_size(obj->header.type & BXON_MASK_TYPE);
        uint64_t readPos = index * natSize;
        void * data = calloc(1,natSize);
        memcpy(data,(uint8_t*)array->objects+readPos,natSize);
        ret = bxon_new();
        ret->header.type = obj->header.type & BXON_MASK_TYPE;
        ret->header.length = natSize;
        ret->data = data;
        return ret;
    }
    return NULL;
}

void bxon_array_release(struct bxon_object * obj){
    struct bxon_data_array * array = obj->data;
    if(array->objects != NULL){
        if(bxon_get_type(obj) == BXON_OBJECT){
            uint32_t i;
            struct bxon_object ** d = (struct bxon_object **)array->objects;
            for(i = 0;i<array->size;i++){
                if(d[i] != NULL)
                    bxon_release(d+i);
            }
            free(d);
        }
        free(array->objects);
    }
    array->objects = NULL;
    array->size = 0;
    array->capacity = 0;
    free(array);
    obj->data = NULL;
}

struct bxon_object * bxon_map_new(uint32_t initCapacity){
    struct bxon_data_map * map;
    struct bxon_object * obj;
    obj = bxon_new();
    obj->header.type = BXON_MAP | BXON_LENGTH_64;
    map = (struct bxon_data_map*)calloc(1,sizeof(struct bxon_data_map));

    obj->data = map;
    map->initCapacity = map->capacity = initCapacity;
    map->objects = calloc(map->initCapacity, sizeof(struct bxon_object *));
    map->keys = calloc(map->initCapacity, sizeof(char *));
    return obj;
}

uint32_t bxon_map_size(struct bxon_object * obj){
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;
    return map->size;
}

uint32_t bxon_map_put(struct bxon_object * obj, const char * key, struct bxon_object * elem){
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;

    if(map->size == map->capacity){
        uint32_t tmpCapacity = map->size + map->initCapacity;
        map->capacity = tmpCapacity;
        map->objects = realloc(map->objects, sizeof(struct bxon_object *) * tmpCapacity);
        map->keys = realloc(map->keys, sizeof(char*) * tmpCapacity);
    }

    ((struct bxon_object **)map->objects)[map->size] = elem;
    map->keys[map->size] = strdup(key);

    map->size++;

    return map->size;
}

struct bxon_object * bxon_map_get_object(struct bxon_object * obj, const char * key){
    uint32_t i;
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;
    for(i = 0; i < map->size; i++){
        if(strcmp(map->keys[i],key) == 0)
            return map->objects[i];
    }
    return NULL;
}

const char * bxon_map_get_key(struct bxon_object * obj, uint32_t index){
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;
    if(index < map->size){
        return map->keys[index];
    }
    return NULL;
}

void bxon_map_release(struct bxon_object * obj){
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;
    if(map->keys!=NULL){
        int32_t i;
        for(i = 0;i<map->size;i++)
            free(map->keys[i]);
        free(map->keys);
    }

    if(map->objects!=NULL){
        int32_t i;
        struct bxon_object ** d = (struct bxon_object **)map->objects;
        for(i = 0;i<map->size;i++)
            bxon_release(d+i);
        free(map->objects);
    }

    map->keys = NULL;
    map->objects = NULL;
    map->size = 0;
    map->initCapacity = 0;
    map->capacity = 0;
    free(map);
    obj->data = NULL;
}

uint32_t bxon_native_size(uint8_t type){
    int tSize = 0;
    if(type == BXON_INT) tSize = sizeof(int32_t);
    if(type == BXON_LONG) tSize = sizeof(int64_t);
    else if(type == BXON_BYTE || type == BXON_BOOLEAN) tSize = sizeof(uint8_t);
    else if(type == BXON_FLOAT) tSize = sizeof(float);
    else if(type == BXON_DOUBLE) tSize = sizeof(double);
    return tSize;
}

uint64_t bxon_write_native(struct bxon_object * obj, struct bxon_context * ctx){
    uint64_t size = obj->header.length;
    uint8_t type = obj->header.type;

    if(type == BXON_STRING)
        type |= bxon_length_flag_for_size(size);

    BXON_WRITE(ctx,sizeof(uint8_t),&type);

    if(type == BXON_STRING)
        bxon_write_length(ctx,type,size);

    return BXON_WRITE(ctx,size,obj->data);
}

uint64_t bxon_write_array(struct bxon_object * obj, struct bxon_context * ctx){
    uint8_t type;
    uint64_t mark_start, mark_end, length;

    struct bxon_data_array * array = (struct bxon_data_array *)obj->data;

    type = (obj->header.type & BXON_MASK_TYPE);

    BXON_WRITE(ctx,sizeof(uint8_t),&obj->header.type);
    bxon_write_length(ctx, obj->header.type,obj->header.length);
    mark_start = BXON_TELL(ctx);

    if(type == BXON_OBJECT){
        uint32_t i = 0;
        for(i = 0;i<array->size;i++){
            struct bxon_object ** o = array->objects;
            bxon_write_object(o[i],ctx);
        }
    }else{
        uint64_t tmpSize = array->size * bxon_native_size(type);
        if(tmpSize > UINT32_MAX){
            return 0;
        }else{
            uint32_t dSize = (uint32_t)tmpSize;
            BXON_WRITE(ctx,dSize,array->objects);
        }
    }

    mark_end = BXON_TELL(ctx);

    if(obj->header.length == 0){
        length = mark_end - mark_start;
        obj->header.length = length;
        BXON_SEEK(ctx,mark_start - bxon_length_bytes(obj->header.type));
        bxon_write_length(ctx, obj->header.type,obj->header.length);
        BXON_SEEK(ctx,mark_end);
    }

    return obj->header.length;
}

uint64_t bxon_write_map(struct bxon_object * obj, struct bxon_context * ctx){
    uint32_t i;
    uint64_t mark_start, mark_end, length;
    struct bxon_data_map * map;

    map = (struct bxon_data_map *)obj->data;

    BXON_WRITE(ctx,sizeof(uint8_t),&obj->header.type);
    bxon_write_length(ctx, obj->header.type,obj->header.length);

    mark_start = BXON_TELL(ctx);

    for(i = 0; i<map->size; i++){
        struct bxon_object * keyObj;
        struct bxon_object ** o = (struct bxon_object **)map->objects;

        keyObj = bxon_new_string(map->keys[i]);
        bxon_write_native(keyObj,ctx);
        bxon_release(&keyObj);

        bxon_write_object(o[i],ctx);
    }

    mark_end = BXON_TELL(ctx);

    if(obj->header.length  == 0){
        length = mark_end - mark_start;

        obj->header.length = length;

        BXON_SEEK(ctx,mark_start - bxon_length_bytes(obj->header.type));

        bxon_write_length(ctx, obj->header.type,obj->header.length);

        BXON_SEEK(ctx,mark_end);
    }

    return obj->header.length;
}


uint64_t bxon_write_object(struct bxon_object * obj, struct bxon_context * ctx){
    if(bxon_is_array(obj))
        return bxon_write_array(obj,ctx);
    else if(bxon_is_map(obj))
        return bxon_write_map(obj,ctx);
    else
        return bxon_write_native(obj,ctx);
}

struct bxon_object * bxon_read_map(struct bxon_context * ctx, uint8_t type){
    int32_t i = 0, capacity = BXON_INIT_CAPACITY;
    uint64_t pos = 0, base, size = 0;
    struct bxon_object * ret = NULL;
    struct bxon_data_map * map = NULL;

    size = bxon_read_length(ctx, type);

    map = (struct bxon_data_map *)calloc(1,sizeof(struct bxon_data_map));
    map->keys = (char**)calloc(capacity,sizeof(char*));
    map->objects = calloc(capacity,sizeof(struct bxon_object*));

    pos = base = BXON_TELL(ctx);

    for(i = 0; pos-base < size;i++){
        if(i >= capacity){
            capacity += BXON_INIT_CAPACITY;
            map->keys = (char**)realloc(map->keys,sizeof(char*) * capacity);
            map->objects = realloc(map->objects,sizeof(struct bxon_object*) * capacity);
        }

        uint8_t keyFlag;
        BXON_READ(ctx,sizeof(uint8_t),&keyFlag);
        struct bxon_object * key = bxon_read_native(ctx, keyFlag);
        map->keys[i] = strdup(key->data);
        bxon_release(&key);

        ((struct bxon_object **)map->objects)[i] = bxon_read_object(ctx);
        pos = BXON_TELL(ctx);
    }

    map->initCapacity = BXON_INIT_CAPACITY;
    map->capacity = capacity;
    map->size = i;

    ret = bxon_new();
    ret->header.type = type;
    ret->header.length = size;
    ret->data = map;

    return ret;
}

struct bxon_object * bxon_read_array(struct bxon_context * ctx, uint8_t type){
    uint64_t size = 0;
    struct bxon_object * ret = NULL;
    struct bxon_data_array * array = NULL;

    size = bxon_read_length(ctx, type);

    array = (struct bxon_data_array *)calloc(1,sizeof(struct bxon_data_array));

    if((type & BXON_MASK_TYPE) == BXON_NIL)
    {
        uint32_t i = 0, capacity = BXON_INIT_CAPACITY;
        uint64_t base = 0, pos = 0;

        array->objects = calloc(sizeof(struct bxon_object*),capacity);

        pos = base = BXON_TELL(ctx);

        for(i = 0; pos-base < size;i++){
            if(i >= capacity){
                capacity += BXON_INIT_CAPACITY;
                array->objects = realloc(array->objects,sizeof(struct bxon_object*) * capacity);
            }

            ((struct bxon_object **)array->objects)[i] = bxon_read_object(ctx);

            pos = BXON_TELL(ctx);
        }

        array->initCapacity = BXON_INIT_CAPACITY;
        array->capacity = capacity;
        array->size = i;
    }
    else{
        array->initCapacity = array->capacity = array->size = (uint32_t) (size / bxon_native_size(type & BXON_MASK_TYPE));
        array->objects = calloc(1,size);
        BXON_READ(ctx, (uint32_t)size, array->objects);
    }

    ret = bxon_new();
    ret->header.type = type;
    ret->header.length = size;
    ret->data = array;

    return ret;
}

struct bxon_object * bxon_read_native(struct bxon_context * ctx, uint8_t type){
    uint32_t length;
    struct bxon_object * ret;

    if((type & BXON_MASK_TYPE) == BXON_STRING)
        length = bxon_read_length(ctx,type);
    else
        length = bxon_native_size(type);

    ret = (struct bxon_object*)malloc(sizeof(struct bxon_object));
    ret->header.type = type;
    ret->header.length = length;
    ret->data = NULL;
    
    if(length > 0){
        ret->data = calloc(1,type == BXON_STRING ? length+1 : length);
        BXON_READ(ctx,length,ret->data);
    }
    
    return ret;
}


struct bxon_object * bxon_read_object(struct bxon_context * ctx){
    uint8_t type;
    BXON_READ(ctx, sizeof(uint8_t), &type);

    switch(type & BXON_MASK_FLAG){
    case BXON_MAP:
        return bxon_read_map(ctx,type);
    case BXON_ARRAY:
        return bxon_read_array(ctx,type);
    case BXON_OBJECT:
        return bxon_read_native(ctx,type);
    default:
        return NULL;
    }
}


