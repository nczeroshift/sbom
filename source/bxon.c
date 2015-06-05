
#include "bxon.h"

#include <stdlib.h>
#include <string.h>

#define BXON_READ(ctx,size,data) (ctx->read)(ctx,size,(uint8_t*)data)
#define BXON_WRITE(ctx,size,data) (ctx->write)(ctx,size,(uint8_t*)data)


void bxon_header_init(struct bxon_header * header){
	//header->name = NULL;
	header->type = 0;
	header->length = 0;
}
void bxon_header_release(struct bxon_header * header){
	//if(header->name!=NULL)
	//	free(header->name);
	//header->name = NULL;
}

uint8_t bxon_header_read(struct bxon_context * context, struct bxon_header * header){
    /*uint8_t keylen;
    if(BXON_READ(context,sizeof(uint8_t),&keylen) != sizeof(uint8_t))
        return 0;

    if(keylen > 0){
        header->name = (char*)malloc(keylen);
        if(BXON_READ(context,keylen,&header->name) != keylen)
            return 0;
    }*/

    if(BXON_READ(context,sizeof(uint8_t),&header->type) != sizeof(uint8_t))
            return 0;

    uint8_t flag = header->type & BXON_MASK_FLAG;
    
    if((header->type & BXON_MASK_TYPE) == BXON_STRING || flag == BXON_MAP || flag == BXON_ARRAY){
        if(flag & BXON_EXTENDED){
            if(BXON_READ(context,sizeof(uint64_t),&header->length) != sizeof(uint64_t))
                return 0;
        }else{
            uint32_t tmp;
            if(BXON_READ(context,sizeof(uint32_t),&tmp) != sizeof(uint32_t))
                return 0;
            header->length = tmp;
        }
    }
    
	return 1;
}

uint32_t bxon_read_string(struct bxon_context * context, struct bxon_header * header, char ** string){
    if(header->type == BXON_STRING){
        *string = calloc(header->length, 1);
        return BXON_READ(context,(uint32_t)header->length,*string);
    }
    return 0;
}

uint8_t bxon_read_int(struct bxon_context * context, struct bxon_header * header, int * value){
    return (header->type & BXON_MASK_TYPE) == BXON_INT && BXON_READ(context,sizeof(int),value) == sizeof(int);
}

uint8_t bxon_read_long(struct bxon_context * context, struct bxon_header * header, int64_t * value){
    return (header->type & BXON_MASK_TYPE) == BXON_LONG && BXON_READ(context,sizeof(int64_t),value) == sizeof(int64_t);
}

uint8_t bxon_read_float(struct bxon_context * context, struct bxon_header * header, float * value){
    return (header->type & BXON_MASK_TYPE) == BXON_FLOAT && BXON_READ(context,sizeof(float),value) == sizeof(float);
}

uint8_t bxon_read_double(struct bxon_context * context, struct bxon_header * header, double * value){
    return (header->type & BXON_MASK_TYPE) == BXON_DOUBLE && BXON_READ(context,sizeof(double),value) == sizeof(double);
}

uint8_t bxon_read_bool(struct bxon_context * context, struct bxon_header * header, uint8_t * value){
    return (header->type & BXON_MASK_TYPE) == BXON_DOUBLE && BXON_READ(context,sizeof(uint8_t),value) == sizeof(uint8_t);
}


uint8_t bxon_parse_node(struct bxon_context * context, struct bxon_header * header, bxon_proto_parse_callback callback){
    
    if( !callback(context, header) ){
        (*context->seek)(context,header->length);
        
        struct bxon_header hdr;
        bxon_header_init(&hdr);
        if( bxon_header_read(context,&hdr) )
            context->error = bxon_parse_node(context,&hdr,callback);
        bxon_header_release(&hdr);
        
        return context->error == BXON_ERROR_NONE;
    }
    
    uint8_t flag = header->type & BXON_MASK_FLAG;
 
    if(flag == BXON_ARRAY && (header->type & BXON_MASK_TYPE) == BXON_OBJECT){
        for( uint64_t i = 0 ; i < header->length ; i++ ){
            
            struct bxon_header hdr;
            bxon_header_init(&hdr);
            if( bxon_header_read(context,&hdr) )
                context->error = bxon_parse_node(context,&hdr,callback);
            bxon_header_release(&hdr);
                
            if(context->error != BXON_ERROR_NONE)
                break;
        }
    }
    else if(flag == BXON_ARRAY){
    }
    else if(flag == BXON_MAP){
        for( uint64_t i = 0 ; i < header->length ; i++ ){
            struct bxon_header keyHdr;
            bxon_header_init(&keyHdr);
            if( bxon_header_read(context,&keyHdr) ){
                /*uint8_t type = keyHdr.type & BXON_MASK_TYPE;
                
                if(type == BXON_STRING ){
                    char * key = NULL;
                    bxon_read_string(context,&keyHdr,&key);
                }else if(type == BXON_INT ){
                    int key = 0;
                    bxon_read_int(context,&keyHdr,&key);
                }else if(type == BXON_LONG){
                    int64_t key = 0;
                    bxon_read_long(context,&keyHdr,&key);
                }
                
                struct bxon_header hdr;
                bxon_header_init(&hdr);
                if( bxon_header_read(context,&hdr) )
                    context->error = bxon_parse_node(context,&hdr,callback);
                bxon_header_release(&hdr);*/

            }
            
            bxon_header_release(&keyHdr);
        }
    }

    return context->error == BXON_ERROR_NONE;
}


uint8_t bxon_parse_document(struct bxon_context * context, bxon_proto_parse_callback callback){
	struct bxon_header hdr;
	bxon_header_init(&hdr);
    if( bxon_header_read(context,&hdr) ){
        if( (hdr.type & BXON_MASK_FLAG) == BXON_MAP || (hdr.type & BXON_MASK_FLAG) == BXON_ARRAY )
            context->error = bxon_parse_node(context,&hdr,callback);
        else
            context->error = BXON_ERROR_ARRAY_OR_MAP_MISSING;
    }
	bxon_header_release(&hdr);
	return context->error == BXON_ERROR_NONE;
}

void bxon_write_header(struct bxon_context * context, struct bxon_header * header){
    //uint8_t keyLen = 0;
    //if(header->name!=NULL)
    //    keyLen = (uint8_t)strlen(header->name);
    //BXON_WRITE(context,sizeof(uint8_t),&keyLen);
    
    //if(header->name!=NULL)
    //     BXON_WRITE(context,keyLen,&header->name);
    
    BXON_WRITE(context,sizeof(uint8_t),&header->type);
    
    uint8_t flag = header->type & BXON_MASK_FLAG;

    if((header->type & BXON_MASK_TYPE) == BXON_STRING || flag == BXON_MAP || flag == BXON_ARRAY){
        if(flag & BXON_EXTENDED){
            BXON_WRITE(context,sizeof(uint64_t),&header->length);
        }else{
            uint32_t tmp = (uint32_t)header->length;
            BXON_WRITE(context,sizeof(uint32_t),&tmp);
        }
    }
}


void bxon_object_init(struct bxon_object * obj){
    bxon_header_init(&obj->header);
}

void bxon_object_release(struct bxon_object * obj){
    bxon_header_release(&obj->header);
}

void bxon_map_init(struct bxon_object * map){
    
}

void bxon_map_release(struct bxon_object * map){
    
}

uint8_t bxon_map_add_object(struct bxon_object * map, struct bxon_object * obj, char * key){
    return 0;
}

uint8_t bxon_map_contains_key(struct bxon_object * map, char * key){
    return 0;
}

struct bxon_object * bxon_map_remove_key(struct bxon_object * map, char * key){
    return NULL;
}

uint32_t bxon_map_size(struct bxon_object * map, char * key){
    return 0;
}

uint8_t bxon_map_get_key(struct bxon_object * map, uint32_t index, char ** key){
    return 0;
}

uint8_t bxon_map_get_object(struct bxon_object * map, char * key){
    return 0;
}

void bxon_vector_init(struct bxon_object * vector, uint8_t type){
    
}

void bxon_vector_release(struct bxon_object * vector){
    
}

uint8_t bxon_vector_get_type(struct bxon_object * vector){
    return 0;
}

void bxon_vector_set_type(struct bxon_object * vector, uint8_t type){
    
}

uint32_t bxon_vector_size(struct bxon_object * vector){
    return 0;
}

struct bxon_object * bxon_vector_get_object(struct bxon_object * vector, uint32_t index){
    return 0;
}

uint8_t bxon_vector_push_object(struct bxon_object * vector, struct bxon_object * obj){
    return 0;
}

uint8_t bxon_vector_put_object(struct bxon_object * vector, uint32_t index, struct bxon_object * obj){
    return 0;
}

uint8_t bxon_vector_put_int(struct bxon_object * vector, uint32_t index, int32_t value){
    return 0;
}

uint8_t bxon_vector_put_float(struct bxon_object * vector, uint32_t index, float value){
    return 0;
}

uint8_t bxon_vector_put_double(struct bxon_object * vector, uint32_t index, double value){
    return 0;
}

uint8_t bxon_vector_put_byte(struct bxon_object * vector, uint32_t index, double value){
    return 0;
}

void bxon_object_make_int(struct bxon_object * obj, int32_t value){
    bxon_object_init(obj);
    obj->header.type = BXON_INT;
    obj->header.length = sizeof(int32_t);
    obj->data = malloc(sizeof(int32_t));
    *((int*)obj->data) = value;
}

void bxon_object_make_long(struct bxon_object * obj, int64_t value){
    bxon_object_init(obj);
    obj->header.type = BXON_LONG;
    obj->header.length = sizeof(int64_t);
    obj->data = malloc(sizeof(int64_t));
    *((int64_t*)obj->data) = value;
}

void bxon_object_make_bool(struct bxon_object * obj, uint8_t value){
    bxon_object_init(obj);
    obj->header.type = BXON_BOOLEAN;
    obj->header.length = sizeof(uint8_t);
    obj->data = malloc(sizeof(uint8_t));
    *((uint8_t*)obj->data) = value;
}

void bxon_object_make_float(struct bxon_object * obj, float value){
    bxon_object_init(obj);
    obj->header.type = BXON_FLOAT;
    obj->header.length = sizeof(float);
    obj->data = malloc(sizeof(float));
    *((float*)obj->data) = value;

}
void bxon_object_make_double(struct bxon_object * obj, double value){
    bxon_object_init(obj);
    obj->header.type = BXON_DOUBLE;
    obj->header.length = sizeof(double);
    obj->data = malloc(sizeof(double));
    *((double*)obj->data) = value;
}

void bxon_object_make_byte(struct bxon_object * obj, uint8_t byte){
    bxon_object_init(obj);
    obj->header.type = BXON_BYTE;
    obj->header.length = sizeof(uint8_t);
    obj->data = malloc(sizeof(uint8_t));
    *((uint8_t*)obj->data) = byte;
}

void bxon_object_make_string(struct bxon_object * obj, char * string){
    
}

uint8_t bxon_object_get_type(struct bxon_object * obj){
    return 0;
}

int32_t bxon_object_get_int(struct bxon_object * obj){
    return 0;
}

int64_t bxon_object_get_long(struct bxon_object * obj){
    return 0;
}

uint8_t bxon_object_get_bool(struct bxon_object * obj){
    return 0;
}

float bxon_object_get_float(struct bxon_object * obj){
    return 0;
}

double bxon_object_get_double(struct bxon_object * obj){
    return 0;
}

uint8_t bxon_object_get_byte(struct bxon_object * obj){
    return 0;
}

int32_t bxon_object_get_string(struct bxon_object * obj, char ** string){
    return 0;
}


