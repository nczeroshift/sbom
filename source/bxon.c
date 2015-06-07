
#include "bxon.h"

#include <stdlib.h>
#include <string.h>

struct bxon_object *    bxon_new            ();
void                    bxon_map_release    (struct bxon_object * obj);
void                    bxon_array_release  (struct bxon_object * obj);

uint32_t                bxon_native_size    (uint8_t type);

struct bxon_object * bxon_new(){
    struct bxon_object * obj = calloc(1,sizeof(struct bxon_object));
    return obj;
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

struct bxon_object * bxon_new_string(char * string){
   struct bxon_object * obj = bxon_new();
   obj->header.type = BXON_BYTE;
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

int32_t bxon_get_string(struct bxon_object * obj, char ** string){
    return 0;
}


struct bxon_object * bxon_array_new(uint8_t nativeType, int initCapacity){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_ARRAY;
    struct bxon_data_array * array = (struct bxon_data_array*)calloc(sizeof(struct bxon_data_array),1);
    obj->data = array;
    if(nativeType == BXON_OBJECT)
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

        if(array->size+1 == array->capacity){
            uint64_t tmpCapacity = array->size + array->initCapacity;
            void * tmp = realloc(array->objects, sizeof(struct bxon_object *) * tmpCapacity);
            if(tmp == NULL)
                return 0;
            array->capacity = tmpCapacity;
            free(array->objects);
            array->objects = tmp;
        }

        ((struct bxon_object **)array->objects)[array->size] = elem;
        array->size++;
    }else{
        uint32_t natSize = bxon_native_size(elem->header.type & BXON_MASK_TYPE);

        if(!natSize)
            return 0;

        if(array->objects == NULL)
            array->objects = calloc(array->initCapacity,natSize);

        if(array->size+1 == array->capacity){
             uint64_t tmpCapacity = array->size + array->initCapacity;
             void * tmp = realloc(array->objects, natSize * tmpCapacity);
             if(tmp == NULL)
                 return 0;
             array->capacity = tmpCapacity;
             free(array->objects);
             array->objects = tmp;
        }

        uint64_t writePos = array->size * natSize;
        memcpy(array->objects+writePos,elem->data,natSize);
        array->size++;
    }
    return array->size;
}

struct bxon_object * bxon_array_get_object(struct bxon_object * obj, int32_t index){
    struct bxon_data_array * array = obj->data;
    if(index >= array->size)
        return NULL;

    if(obj->header.type == BXON_OBJECT){
        return ((struct bxon_object **)array->objects)[index];
    }else{
        uint32_t natSize = bxon_native_size(obj->header.type & BXON_MASK_TYPE);
        uint64_t readPos = index * natSize;
        void * data = calloc(1,natSize);
        memcpy(data,array->objects+readPos,natSize);
        struct bxon_object * ret = bxon_new();
        ret->header.type = obj->header.type & BXON_MASK_TYPE;
        ret->data = data;
        return ret;
    }
    return NULL;
}

void bxon_array_release(struct bxon_object * obj){
    struct bxon_data_array * array = obj->data;
    if(array->objects != NULL){
        if(bxon_get_type(obj) == BXON_OBJECT){
            struct bxon_object ** d = (struct bxon_object **)array->objects;
            for(int32_t i = 0;i<array->size;i++){
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

struct bxon_object * bxon_map_new(int initCapacity){
    struct bxon_object * obj = bxon_new();
    obj->header.type = BXON_MAP;
    struct bxon_data_map * map = (struct bxon_data_map*)calloc(1,sizeof(struct bxon_data_map));
    obj->data = map;
    map->capacity = initCapacity;
    return obj;
}

uint32_t bxon_map_size(struct bxon_object * obj){
    struct bxon_data_map * map = (struct bxon_data_map *)obj->data;
    return map->size;
}

uint32_t bxon_map_put(struct bxon_object * obj, const char * key, struct bxon_object * elem){
    return 0;
}

struct bxon_object *bxon_map_get_object(struct bxon_object * obj, const char * key){
    return NULL;
}

const char *bxon_map_get_key(struct bxon_object * obj, int32_t index){
    return NULL;
}

void bxon_map_release(struct bxon_object * obj){

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
