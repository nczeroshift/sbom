
/**
 * BXON - Binary eXchange Object Notation
 * Under MIT License
 * Copyright © Luís F. Loureiro
 * https://github.com/nczeroshift/bxon
 * Version ??.??.??
 * 2015-08-05
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "bxon.h"
#include "bxon.hpp"

uint32_t cFIORead(struct bxon_context * context, int32_t size, uint8_t * d){
    FILE * f = (FILE*)context->data;
    return fread((void*)d,1,(size_t)size,f);
}

uint32_t cFIOWrite(struct bxon_context * context, int32_t size, uint8_t * d){
    FILE * f = (FILE*)context->data;
    return fwrite((void*)d,1,(size_t)size,f);
}

uint64_t cFIOSeek(struct bxon_context * context, int64_t offset){
    FILE * f = (FILE*)context->data;
    return fseek(f,offset,SEEK_SET);
}

uint64_t cFIOTell(struct bxon_context * context){
    FILE * f = (FILE*)context->data;
    return ftell(f);
}


class cppFIOContext : public BXON::Context{
public:
    cppFIOContext(std::string filename){
        file = fopen(filename.c_str(),"rb");
    }
    
    virtual ~cppFIOContext(){
        if(file!=NULL)
            fclose(file);
    }
    
    uint32_t Read(int32_t size, uint8_t * d){
        if(fread((void*)d, 1, (size_t)size, file)==size)
            return size;
        return 0;
    }
    
    uint32_t Write(int32_t size, uint8_t * d){
        if(fwrite((void*)d, 1, (size_t)size, file)==size)
            return size;
        return 0;
    }
    
    uint64_t Seek(int64_t offset){
        return fseek(file, offset, SEEK_SET);
    }
    
    uint64_t Tell(){
        return ftell(file);
    }
    
protected:
    FILE * file;
};

void printspaces(int depth){
    for(int i = 0;i<depth*4;i++)
        printf(" ");
}
void cPrintObject(struct bxon_object * obj , int depth = 0){
    if(bxon_is_array(obj)){
        printf("[\n");
        
        for(int i = 0;i<bxon_array_size(obj);i++){
            printspaces(depth+1);
            if(i>0)
                printf(",");
            struct bxon_object * obj2 = bxon_array_get_object(obj, i);
            cPrintObject(obj2,depth+1);
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
            cPrintObject(obj2,depth+1);
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


void cppPrintObject(BXON::Object * obj, int depth = 0){
    BXON::Array * array=NULL;
    BXON::Map * map = NULL;
    BXON::Native * native = NULL;
    
    if((array = dynamic_cast<BXON::Array*>(obj)) != NULL){
        printf("[\n");
        if(array->GetType()){
            int columns = 0;
            if(array->GetSize() % 3 == 0)
                columns = 3;
            else if(array->GetSize() % 4 == 0)
                columns = 4;
            
            for(int i = 0, c = 0;i<array->GetSize();i++){
                if(c == 0)
                    printspaces(depth+1);
                
                if(i>0)
                    printf(",");
                if(array->GetType() == BXON::TYPE_INT){
                    printf("%d",array->GetInteger(i));
                }else if(array->GetType() == BXON::TYPE_LONG){
                    printf("%lld",array->GetLong(i));
                }else if(array->GetType() == BXON::TYPE_FLOAT){
                    printf("%f",array->GetFloat(i));
                }else if(array->GetType() == BXON::TYPE_DOUBLE){
                    printf("%f",array->GetDouble(i));
                }else if(array->GetType() == BXON::TYPE_BYTE){
                    printf("0x%x",array->GetByte(i));
                }else if(array->GetType() == BXON::TYPE_BOOLEAN){
                    printf("%s",native->GetBoolean()?"true":"false");
                }
                
                if(c >= columns-1){
                    c = 0;
                    printf("\n");
                }else
                    c++;
            }
        }else{
            for(int i = 0;i<array->GetSize();i++){
                BXON::Object * nObj = array->GetObject(i);
                printspaces(depth+1);
                if(i>0)
                    printf(",");
                if(obj!=NULL)
                    cppPrintObject(nObj,depth+1);
                else
                    printf("null");
            }
        }
        printspaces(depth+1);printf("]\n");
    }else if((map = dynamic_cast<BXON::Map*>(obj)) != NULL){
        printf("{\n");
        std::vector<std::string> keys = map->GetKeys();
        for(int i = 0;i<map->GetSize();i++){
            printspaces(depth+1);
            if(i>0)
                printf(",");
            printf("\"%s\":",keys[i].c_str());
            BXON::Object * nObj = map->GetObject(keys[i]);
            cppPrintObject(nObj,depth+1);
        }
        printspaces(depth);printf("}\n");
    }else if((native = dynamic_cast<BXON::Native*>(obj)) != NULL){
        
        if(native->GetType() == BXON::TYPE_NIL){
            printf("nil\n");
        }
        else if(native->GetType() == BXON::TYPE_STRING){
            printf("\"%s\"\n",native->GetString().c_str());
        }
        else if(native->GetType() == BXON::TYPE_INT){
            printf("%d\n",native->GetInt());
        }
        else if(native->GetType() == BXON::TYPE_LONG){
            printf("%lld\n",native->GetLong());
        }
        else if(native->GetType() == BXON::TYPE_BOOLEAN){
            printf("%s\n",native->GetBoolean()?"true":"false");
        }
        else if(native->GetType() == BXON::TYPE_BYTE){
            printf("0x%x\n",native->GetByte());
        }
        else if(native->GetType() == BXON::TYPE_FLOAT){
            printf("%f\n",native->GetFloat());
        }
        else if(native->GetType() == BXON::TYPE_DOUBLE){
            printf("%f\n",native->GetDouble());
        }
    }
}


#define TEST_FILE_PATH "../../../../test.bxon"

void cWriteTest(){
    // Init context
    bxon_context ctx;
    ctx.read = cFIORead;
    ctx.write = cFIOWrite;
    ctx.seek = cFIOSeek;
    ctx.tell = cFIOTell;
    
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

void cReadTest(){
    // Init context
    bxon_context ctx;
    ctx.read = cFIORead;
    ctx.write = cFIOWrite;
    ctx.seek = cFIOSeek;
    ctx.tell = cFIOTell;
    
    FILE *f2 = fopen(TEST_FILE_PATH,"rb");
    if(!f2){
        fprintf(stderr,"File not found!\n");
        return;
    }
    
    ctx.data = f2;
    struct bxon_object * obj = bxon_read_object(&ctx);
    cPrintObject(obj);
    fclose(f2);
}

void cppReadTest(){
    cppFIOContext * ctx = new cppFIOContext(TEST_FILE_PATH);
    
    try{
        BXON::Object * root = BXON::Object::Parse(ctx);
        if(root!=NULL){
            cppPrintObject(root);
            delete root;
        }
    }catch(int e){
        printf("Exception %d",e);
    }
    
    delete ctx;
}


int main(int argv, char * argc[])
{
    cWriteTest();
    cReadTest();
    cppReadTest();
    
#ifdef _DEBUG && _WIN32 
    system("pause");
#endif
    return 0;
}
