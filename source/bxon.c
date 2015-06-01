
#include "bxon.h"

#include <stdlib.h>
#include <string.h>

#define BXON_READ(ctx,size,data) (*ctx->read)(ctx,size,(uint8_t*)data)

void bxon_init(struct bxon_header * header){
	header->name = NULL;
	header->type = 0;
	header->length = 0;
}
void bxon_release(struct bxon_header * header){
	if(header->name!=NULL)
		free(header->name);
	header->name = NULL;
}

uint8_t bxon_header_read(struct bxon_context * context, struct bxon_header * header){
	uint8_t keylen;
	if(BXON_READ(context,sizeof(uint8_t),&keylen) != sizeof(uint8_t))
		return 0;

	if(keylen > 0){
		header->name = (char*)malloc(keylen);
		if(BXON_READ(context,keylen,&header->name) != keylen)
			return 0;
	}

	if(BXON_READ(context,sizeof(uint8_t),&header->type) != sizeof(uint32_t))
			return 0;

	if(header->type & BXON_EXTENDED){
		if(BXON_READ(context,sizeof(uint64_t),&header->length) != sizeof(uint64_t))
			return 0;
	}else{
		uint32_t tmp;
		if(BXON_READ(context,sizeof(uint32_t),&tmp) != sizeof(uint32_t))
			return 0;
		header->length = tmp;
	}
	return 1;
}

uint8_t bxon_parse_node(struct bxon_context * context, struct bxon_header * header, bxon_proto_parse_callback callback){
	uint8_t ret = callback(context,header);
	if(ret == 1){
		// read next
	}
	
	// jump header end
	
}


uint8_t bxon_parse_document(struct bxon_context * context, bxon_proto_parse_callback callback){
	uint8_t ret = 0;
	struct bxon_header hdr;
	bxon_header_init(&hdr);
	if( bxon_header_read(context,&hdr) )
		bxon_parse_node(context,&hdr,callback);
	bxon_header_release(&hdr);
	return 1;
}
