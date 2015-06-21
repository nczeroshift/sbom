
#include "bxon.h"

#include <stdlib.h>
#include <string.h>

#define BXON_READ(ctx,size,data)    (ctx->read) (ctx,size,(uint8_t*)data)
#define BXON_WRITE(ctx,size,data)   (ctx->write)(ctx,size,(uint8_t*)data)
#define BXON_TELL(ctx)              (ctx->tell) (ctx)
#define BXON_SEEK(ctx,pos)          (ctx->seek) (ctx,pos)

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
    obj->header.type =BXON_ARRAY | BXON_EXTENDED;
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
        uint64_t writePos;
		uint32_t natSize = bxon_native_size(elem->header.type & BXON_MASK_TYPE);

        if(!natSize)
            return 0;

        if(array->objects == NULL)
            array->objects = calloc(array->initCapacity,natSize);

        if(array->size == array->capacity){
             uint32_t tmpCapacity = array->size + array->initCapacity;
             void * tmp = realloc(array->objects, natSize * tmpCapacity);
             if(tmp == NULL)
                 return 0;
             array->capacity = tmpCapacity;
             free(array->objects);
             array->objects = tmp;
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

    if(obj->header.type == BXON_OBJECT){
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
    obj->header.type = BXON_MAP | BXON_EXTENDED;
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
		void * tmp, * tmp2;
        uint32_t tmpCapacity = map->size + map->initCapacity;
        
        tmp = realloc(map->objects, sizeof(struct bxon_object *) * tmpCapacity);
        tmp2 = realloc(map->keys, sizeof(char*) * tmpCapacity);
        if(tmp == NULL || tmp2 == NULL)
            return 0;
        map->capacity = tmpCapacity;
        free(map->objects);
        free(map->keys);
        map->objects = tmp;
        map->keys = tmp2;
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
    uint32_t size;
	uint8_t type = obj->header.type;
    BXON_WRITE(ctx,sizeof(uint8_t),&type);
    size = (uint32_t)obj->header.length;
    if(type == BXON_STRING)
        BXON_WRITE(ctx,sizeof(uint32_t),&size);
    return BXON_WRITE(ctx,size,obj->data);
}

uint64_t bxon_write_array(struct bxon_object * obj, struct bxon_context * ctx){
    uint8_t type;
	uint64_t mark_start, mark_end, length;

	struct bxon_data_array * array = (struct bxon_data_array *)obj->data;

    type = (obj->header.type & BXON_MASK_TYPE);

    BXON_WRITE(ctx,sizeof(uint8_t),&obj->header.type);
    BXON_WRITE(ctx,sizeof(uint64_t),&obj->header.length);

    mark_start = BXON_TELL(ctx);

    if(type == BXON_OBJECT){
		uint32_t i = 0;
		uint64_t * positions;
        uint64_t mark_array_end;

		BXON_WRITE(ctx,sizeof(uint32_t),&array->size);
        for(i = 0;i<array->size;i++){
            uint64_t tmp = 0;
            BXON_WRITE(ctx,sizeof(uint64_t),&tmp);
        }

        positions = (uint64_t*)calloc(array->size,sizeof(uint64_t));

        for(i = 0;i<array->size;i++){
            struct bxon_object ** o = array->objects;
            positions[i] = BXON_TELL(ctx);
            bxon_write_object(o[i],ctx);
        }

        free(positions);

        mark_array_end = BXON_TELL(ctx);

        BXON_SEEK(ctx,mark_start+sizeof(uint32_t));
        for(i = 0;i<array->size;i++){
            uint64_t tmp = positions[i] - positions[0];
            BXON_WRITE(ctx,sizeof(uint64_t),&tmp);
        }

        BXON_SEEK(ctx,mark_array_end);
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

    length = mark_end - mark_start;

    obj->header.length = length;

    BXON_SEEK(ctx,mark_start - sizeof(uint64_t));

    BXON_WRITE(ctx,sizeof(uint64_t),&obj->header.length);

    BXON_SEEK(ctx,mark_end);

    return length;
}

uint64_t bxon_write_map(struct bxon_object * obj, struct bxon_context * ctx){
    uint32_t i;
	uint64_t mark_start, mark_end, length;
	uint64_t * positions;
	struct bxon_data_map * map;

	map = (struct bxon_data_map *)obj->data;
	
    BXON_WRITE(ctx,sizeof(uint8_t),&obj->header.type);
    BXON_WRITE(ctx,sizeof(uint64_t),&obj->header.length);

    mark_start = BXON_TELL(ctx);

    BXON_WRITE(ctx,sizeof(uint32_t),&map->size);

    // Write dummy positions
    for(i = 0;i<map->size;i++){
       uint64_t tmp = 0;
       BXON_WRITE(ctx,sizeof(uint64_t),&tmp);
    }

    // Write keys
    for(i = 0;i<map->size;i++){
        uint32_t str = strlen(map->keys[i]);
        BXON_WRITE(ctx,sizeof(uint32_t),&str);
        BXON_WRITE(ctx,str,map->keys[i]);
    }

    // Write objects
    positions = (uint64_t*)calloc(map->size,sizeof(uint64_t));
    for(i = 0;i<map->size;i++){
        struct bxon_object ** o = map->objects;
        positions[i] = BXON_TELL(ctx);
        bxon_write_object(o[i],ctx);
    }

    mark_end = BXON_TELL(ctx);

    // Rewind to beginning and set the correct offsets
    BXON_SEEK(ctx,mark_start + sizeof(uint32_t));

    for(i = 0;i < map->size;i++){
        uint64_t tmp = positions[i] - positions[0];
        BXON_WRITE(ctx,sizeof(uint64_t),&tmp);
    }

    free(positions);

    length = mark_end - mark_start;

    obj->header.length = length;

    BXON_SEEK(ctx,mark_start - sizeof(uint64_t));

    BXON_WRITE(ctx,sizeof(uint32_t),&obj->header.length);

    BXON_SEEK(ctx,mark_end);

    return length;
}


uint64_t bxon_write_object(struct bxon_object * obj, struct bxon_context * ctx){
    if(bxon_is_array(obj))
        return bxon_write_array(obj,ctx);
    else if(bxon_is_map(obj))
        return bxon_write_map(obj,ctx);
    else
        return bxon_write_native(obj,ctx);
}

struct bxon_object * bxon_read_map(struct bxon_context * ctx, uint8_t type, uint64_t size){
    uint32_t i;
	uint32_t count;
    uint64_t base;
	struct bxon_object * ret;
    struct bxon_data_map * map = (struct bxon_data_map *)calloc(1,sizeof(struct bxon_data_map));
    
    BXON_READ(ctx, sizeof(uint32_t), &count);
    map->offset = (uint64_t*)calloc(sizeof(uint64_t),count);
    for(i = 0;i<count;i++){
        BXON_READ(ctx, sizeof(uint64_t), &map->offset[i]);
    }
    map->keys = (char**)calloc(sizeof(char*),count);
    for(i = 0;i<count;i++){
        uint32_t s = 0;
		char * str;
        BXON_READ(ctx, sizeof(uint32_t), &s);
        str = (char*)calloc(1,s+1);
        BXON_READ(ctx, s, str);
        map->keys[i] = str;
    }
    map->objects = calloc(sizeof(struct bxon_object*),count);
    
    base = BXON_TELL(ctx);
    for(i = 0;i<count;i++){
        BXON_SEEK(ctx, base+map->offset[i]);
        ((struct bxon_object **)map->objects)[i] = bxon_read_object(ctx);
    }
    map->initCapacity = map->capacity = map->size = count;
	ret = bxon_new();
    ret->header.type = type;
    ret->header.length = size;
    ret->data = map;
    
    return ret;
}

struct bxon_object * bxon_read_array(struct bxon_context * ctx, uint8_t type, uint64_t size){
    uint32_t count;
    struct bxon_object * ret;
    struct bxon_data_array * array = (struct bxon_data_array *)calloc(1,sizeof(struct bxon_data_array));
   

    if((type & BXON_MASK_TYPE) == BXON_OBJECT)
    {
		uint32_t i;
		uint64_t base;

        BXON_READ(ctx, sizeof(uint32_t), &count);
        
        array->offset = (uint64_t*)calloc(sizeof(uint64_t),count);
        for(i = 0;i<count;i++){
            BXON_READ(ctx, sizeof(uint64_t), &array->offset[i]);
        }
        
        array->objects = calloc(sizeof(struct bxon_object*),count);
        base = BXON_TELL(ctx);
        for(i = 0;i<count;i++){
            BXON_SEEK(ctx, base+array->offset[i]);
            ((struct bxon_object **)array->objects)[i] = bxon_read_object(ctx);
        }
    }
    else{
        array->initCapacity = array->capacity = array->size = (uint32_t) (size / bxon_native_size(type & BXON_MASK_TYPE));
        array->objects = calloc(1,size);
        BXON_READ(ctx,(uint32_t) size, array->objects);
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

    if(type == BXON_STRING)
        BXON_READ(ctx, sizeof(uint32_t), &length);
    else
        length = bxon_native_size(type);
    
    ret = (struct bxon_object*)malloc(sizeof(struct bxon_object));
    ret->header.type = type;
    ret->header.length = length;
    ret->data = calloc(1,type == BXON_STRING ? length+1 : length);
    
    BXON_READ(ctx,length,ret->data);
    
    return ret;
}


struct bxon_object * bxon_read_object(struct bxon_context * ctx){
    uint8_t type;
    uint8_t flag;
    BXON_READ(ctx, sizeof(uint8_t), &type);
    
    flag = type & BXON_MASK_FLAG;
    
    if( (flag & BXON_MAP) != 0){
        uint64_t size;
        BXON_READ(ctx, sizeof(uint64_t), &size);
        return bxon_read_map(ctx,type,size);
    }
    else if((flag & BXON_ARRAY) != 0){
        uint64_t size;
        BXON_READ(ctx, sizeof(uint64_t), &size);
        return bxon_read_array(ctx,type,size);
    }else{
        return bxon_read_native(ctx,type);
    }
    
    return NULL;
}


