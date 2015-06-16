
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "bxon.h"

uint32_t fio_read(struct bxon_context * context, int32_t size, uint8_t * d){
    FILE * f = (FILE*)context->data;
    return fread((void*)d,1,(size_t)size,f);
}

uint32_t fio_write(struct bxon_context * context, int32_t size, uint8_t * d){
    FILE * f = (FILE*)context->data;
    return fwrite((void*)d,1,(size_t)size,f);
}

uint64_t fio_seek(struct bxon_context * context, int64_t offset){
    FILE * f = (FILE*)context->data;
    return fseek(f,offset,SEEK_SET);
}

uint64_t fio_tell(struct bxon_context * context){
    FILE * f = (FILE*)context->data;
    return ftell(f);
}

int main(int argv, char * argc[])
{
    bxon_context * ctx = (bxon_context*)malloc(sizeof(bxon_context));
    ctx->read = fio_read;
    ctx->write = fio_write;
    ctx->seek = fio_seek;
    ctx->tell = fio_tell;
    
    FILE *f = fopen("out.bin","wb");
    ctx->data = f;
    bxon_object * map = bxon_map_new(10);

    bxon_map_put(map,"int",bxon_new_int(10));
    bxon_map_put(map,"long",bxon_new_long(1e8));
    bxon_map_put(map,"string",bxon_new_string("bxon test string"));
    bxon_map_put(map,"float",bxon_new_float(10.000001f));
    bxon_map_put(map,"double",bxon_new_double(12.000001f));
    bxon_map_put(map,"boolean",bxon_new_bool(0));
    bxon_map_put(map,"bool",bxon_new_bool(0));
    bxon_map_put(map,"byte",bxon_new_byte(0xFA));

    bxon_object * array = bxon_array_new(BXON_FLOAT,3);
    bxon_array_push(array,bxon_new_float(0.01f));
    bxon_array_push(array,bxon_new_float(0.02f));
    bxon_array_push(array,bxon_new_float(0.03f));

    bxon_map_put(map,"array",array);

    bxon_write_object(map,ctx);
    
    bxon_release(&map);

    fclose(f);
    
    FILE *f2 = fopen("out.bin","rb");
    ctx->data = f2;
    bxon_read_object(ctx);
    fclose(f);
    
	return 0;
}
