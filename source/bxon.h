
#ifndef BXON_H_
#define BXON_H_

#include <stdint.h>

typedef enum bxon_types{
	BXON_STRING = 1,
	BXON_BOOLEAN = 2,
	BXON_INT = 3,
	BXON_LONG = 4, 
	BXON_FLOAT = 8,
	BXON_DOUBLE = 16,
	BXON_MAP = 32,
	BXON_ARRAY = 64,
	BXON_EXTENDED = 1024,
};

struct bxon_context;
struct bxon_header;

typedef int32_t (*bxon_proto_read) (struct bxon_context * c, int s, uint8_t * d);
typedef int32_t (*bxon_proto_write) (struct bxon_context * c, int s, uint8_t * d);

typedef int8_t (*bxon_proto_parse_callback) (struct bxon_context * c,struct bxon_header * h);


struct bxon_context{
	bxon_proto_read * read;
	bxon_proto_write * write;
	void * priv;
};

struct bxon_header{
	char * name;
	uint32_t type;
	uint64_t length;
};

void bxon_header_init(struct bxon_header * header);
void bxon_header_release(struct bxon_header * header);

uint8_t bxon_header_read(struct bxon_context * context, struct bxon_header * header);

uint8_t bxon_parse_document(struct bxon_context * context, bxon_proto_parse_callback callback);

#endif