
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "bxon.h"

int32_t io_write(struct bxon_context * context, int32_t size, uint8_t * d){
    return fwrite((void*)d,(size_t)size,(size_t)1,(FILE*)context->priv);
}

int32_t io_read(struct bxon_context * context, int32_t size, uint8_t * d){
    return fread((void*)d,(size_t)size,(size_t)1,(FILE*)context->priv);
}

int8_t parse (struct bxon_context * context,struct bxon_header * header){
    
    return 0;
}

int main(int argv, char * argc[])
{
    /*bxon_context * ctx = (bxon_context*)calloc(sizeof(bxon_context),1);
    ctx->priv = (void*)fopen("temp.bxon","wb");
    ctx->write = (bxon_proto_write*)io_write;
    ctx->read = (bxon_proto_write*)io_read;
    
    bxon_header temp;
    bxon_header_init(&temp);
    temp.type = BXON_MAP;
    //temp.name = NULL;
    temp.length = 1;
    
    bxon_write_header(ctx,&temp);
    
    
    
    fclose((FILE*)ctx->priv);
    
    ctx->priv = (void*)fopen("temp.bxon","rb");
    bxon_parse_document(ctx,parse);
    fclose((FILE*)ctx->priv);*/
    
	return 0;
}