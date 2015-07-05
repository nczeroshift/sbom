
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

void printspaces(int depth){
    for(int i = 0;i<depth*4;i++)
        printf(" ");
}
void print_object(struct bxon_object * obj , int depth){
    if(bxon_is_array(obj)){
        printf("[\n");

        for(int i = 0;i<bxon_array_size(obj);i++){
            printspaces(depth+1);
            if(i>0)
                printf(",");
            struct bxon_object * obj2 = bxon_array_get_object(obj, i);
            print_object(obj2,depth+1);
        }

        printspaces(depth);printf("]\n");
    }else if(bxon_is_map(obj)){
        printf("{\n");
        for(int i = 0;i<bxon_map_size(obj);i++){
            printspaces(depth+1);

            if(i>0)
                printf(",");

            const char * key = bxon_map_get_key(obj, i);
            printf("\"%s\":",key);
            struct bxon_object * obj2 = bxon_map_get_object(obj, key);
            print_object(obj2,depth+1);
        }
        printspaces(depth);printf("}\n");
    }else{
        switch(bxon_get_type(obj)){
        case BXON_NIL:
            printf("nil\n");
            break;
        case BXON_INT:
            printf("%d\n",bxon_get_int(obj));
            break;
        case BXON_FLOAT:
            printf("%f\n",bxon_get_float(obj));
            break;
        case BXON_BYTE:
            printf("%x\n",bxon_get_byte(obj));
            break;
        case BXON_BOOLEAN:
            printf("%s\n",bxon_get_bool(obj)?"true":"false");
            break;
        case BXON_DOUBLE:
            printf("%f\n",bxon_get_double(obj));
            break;
        case BXON_STRING:
            printf("\"%s\"\n",bxon_get_string(obj));
            break;
        case BXON_LONG:
            printf("%lld\n",bxon_get_long(obj));
            break;

        }
    }
}

#define TEST_FILE_PATH "test.bxon"

void write_test(){
	// Init context
	bxon_context ctx;
    ctx.read = fio_read;
    ctx.write = fio_write;
    ctx.seek = fio_seek;
    ctx.tell = fio_tell;

	// Open file to writing and assing to context
	FILE *f = fopen(TEST_FILE_PATH,"wb");
    ctx.data = f;

	// Create root objet (map or array)
    bxon_object * map = bxon_map_new(10);

    bxon_map_put(map,"int_1",bxon_new_int(10));
    bxon_map_put(map,"long_2",bxon_new_long(1e8));
    bxon_map_put(map,"string_3",bxon_new_string("bxon test string"));
    bxon_map_put(map,"float_4",bxon_new_float(10.000001f));
    bxon_map_put(map,"double_5",bxon_new_double(12.000001f));
    bxon_map_put(map,"boolean_6",bxon_new_bool(0));
    bxon_map_put(map,"bool_7",bxon_new_bool(0));
    bxon_map_put(map,"byte_8",bxon_new_byte(0xBA));
    bxon_map_put(map,"nil_9",bxon_new_nil());
    
    bxon_object * map2 = bxon_map_new(20);
    for(int i = 0;i<20;i++){
        char str[128] = "";
        sprintf(str,"v_%d_test_value",i);
        bxon_map_put(map2,str,bxon_new_int(i*1000));
    }

    bxon_map_put(map,"map_10",map2);

    bxon_object * array = bxon_array_new(BXON_FLOAT,3);
    bxon_array_push(array,bxon_new_float(0.01f));
    bxon_array_push(array,bxon_new_float(0.02f));
    bxon_array_push(array,bxon_new_float(0.03f));

    bxon_map_put(map,"array_11",array);

    bxon_write_object(map,&ctx);

    bxon_release(&map);

    fclose(f);
}

void read_test(){
	// Init context
	bxon_context ctx;
	ctx.read = fio_read;
    ctx.write = fio_write;
    ctx.seek = fio_seek;
    ctx.tell = fio_tell;

	FILE *f2 = fopen(TEST_FILE_PATH,"rb");
	if(!f2){
		fprintf(stderr,"File not found!\n");
		return;
	}

    ctx.data = f2;
    struct bxon_object * obj = bxon_read_object(&ctx);
    print_object(obj,0);
    fclose(f2);
}

int main(int argv, char * argc[])
{
	write_test();
	read_test();

#ifdef _DEBUG && _WIN32 
	system("pause");
#endif
	return 0;
}
