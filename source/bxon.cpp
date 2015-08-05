
/**
 * BXON - Binary eXchange Object Notation
 * Under MIT License
 * Copyright © Luís F. Loureiro
 * https://github.com/nczeroshift/bxon
 * Version ??.??.??
 * 2015-08-05
 */

#include "bxon.hpp"
#include <string.h>
#include <stdlib.h>

_BXON_BEGIN

class ObjectArray : public Array{
public:
    ObjectArray();
    ObjectArray(int capacity);
    virtual ~ObjectArray();
    
    uint32_t GetSize();
    uint32_t GetCapacity();
    
    Object * GetObject (int i);
    Array * GetArray (int i);
    Map * GetMap (int i);
    
    bool IsNil (int i);
    bool GetBoolean (int i);
    uint8_t GetByte (int i);
    int32_t GetInteger (int i);
    int64_t GetLong (int i);
    float GetFloat (int i);
    double GetDouble (int i);
    std::string GetString (int i);
    
protected:
    void Read(const Header & hdr,Context * ctx);
    std::vector<Object*> array;
};

class NativeArray : public Array{
public:
    NativeArray(NativeType type, uint32_t capacity);
    virtual ~NativeArray();
    
    uint32_t GetSize();
    uint32_t GetCapacity();
    
    Object * GetObject (int i);
    Array * GetArray (int i);
    Map * GetMap (int i);
    
    bool IsNil (int i);
    bool GetBoolean (int i);
    uint8_t GetByte (int i);
    int32_t GetInteger (int i);
    int64_t GetLong (int i);
    float GetFloat (int i);
    double GetDouble (int i);
    std::string GetString (int i);
    
protected:
    void Read(const Header & hdr,Context * ctx);
    uint32_t nativeLen;
    uint32_t capacity;
    uint32_t size;
    void * data;
};

uint8_t Object::SizeForNative(NativeType type){
    switch(type){
        case TYPE_NIL:
            return 0;
        case TYPE_BYTE:
            return sizeof(uint8_t);
        case TYPE_BOOLEAN:
            return sizeof(bool);
        case TYPE_INT:
            return sizeof(int32_t);
        case TYPE_LONG:
            return sizeof(int64_t);
        case TYPE_FLOAT:
            return sizeof(float);
        case TYPE_DOUBLE:
            return sizeof(double);
        default:
            return 0;
    }
}

Header::Header(){
    type = 0;
    length = 0;
}

Header::Header(const Header & hdr){
    type = hdr.type;
    length = hdr.length;
}

Header::~Header()
{
}

void Header::SetType(NativeType t){
    type = (type & ~(BXON_MASK_TYPE)) | (t & BXON_MASK_TYPE);
}

NativeType Header::GetType(){
    return (NativeType)(type & BXON_MASK_TYPE);
}

LengthFlag Header::GetLenFlag(){
    return (LengthFlag)(type & BXON_MASK_LENGTH);
}

void Header::SetFlag(ObjectFlag flag){
    type = (type & ~(BXON_MASK_FLAG)) | (flag & BXON_MASK_FLAG);
}

ObjectFlag Header::GetFlag(){
    return (ObjectFlag)(type & BXON_MASK_FLAG);
}

void Header::SetLength(int64_t len){
    if(GetFlag() != FLAG_OBJECT || GetType() == TYPE_STRING)
        length = len;
    
    if(length > UINT8_MAX)
        type = (type & ~(BXON_MASK_TYPE)) | LENGTH_16;
    else if(length > UINT16_MAX)
        type = (type & ~(BXON_MASK_TYPE)) | LENGTH_32;
    else if(length > UINT32_MAX)
        type = (type & ~(BXON_MASK_TYPE)) | LENGTH_32;
    else
        type = (type & ~(BXON_MASK_TYPE));
}

int64_t Header::GetLength(){
    if(GetFlag() == FLAG_OBJECT && GetType() != TYPE_STRING){
        return Object::SizeForNative(GetType());
    }
    return length;
}

void Header::Read(Context * context){
    if(context->Read(sizeof(uint8_t), &type)!=sizeof(uint8_t))
        throw EXCEPTION_READ_FAILURE;
    if(GetType() == TYPE_STRING || GetFlag() != FLAG_OBJECT){
        if(GetLenFlag() == LENGTH_8){
            uint8_t tmp;
            if(context->Read(sizeof(uint8_t), (uint8_t*)&tmp) != sizeof(uint8_t))
                throw EXCEPTION_READ_FAILURE;
            length = tmp;
        }else if(GetLenFlag() == LENGTH_16){
            uint16_t tmp;
            if(context->Read(sizeof(uint16_t), (uint8_t*)&tmp) != sizeof(uint16_t))
                throw EXCEPTION_READ_FAILURE;
            length = tmp;
        }else if(GetLenFlag() == LENGTH_32){
            uint32_t tmp;
            if(context->Read(sizeof(uint32_t), (uint8_t*)&tmp) != sizeof(uint32_t))
                throw EXCEPTION_READ_FAILURE;
            length = tmp;
        }else if(GetLenFlag() == LENGTH_64){
            if(context->Read(sizeof(uint64_t), (uint8_t*)&length) != sizeof(uint64_t))
                throw EXCEPTION_READ_FAILURE;
        }
        
    }else{
        length = GetLength();
    }
}

/*
 void Header::Write(Context * context){
 if(context->Write(sizeof(uint8_t), &type)!=sizeof(uint8_t))
 throw EXCEPTION_WRITE_FAILURE;
 if(GetType() == TYPE_STRING || GetType() != FLAG_OBJECT){
 if(context->Write(sizeof(uint64_t), (uint8_t*)&length)!=sizeof(uint64_t))
 throw EXCEPTION_WRITE_FAILURE;
 }
 }
 */

Object::Object(){
    
}

Object::~Object(){
    
}

NativeType Object::GetType(){
    return header.GetType();
}

ObjectFlag Object::GetFlag(){
    return header.GetFlag();
}

int64_t Object::GetLength(){
    return header.GetLength();
}

Object * Object::Parse(Context * ctx){
    Object * ret = NULL;
    Header hdr;
    hdr.Read(ctx);
    if(hdr.GetFlag() == FLAG_ARRAY){
        NativeType type = hdr.GetType();
        if(type!=0){
            int64_t count = hdr.GetLength() / Native::SizeForNative(type);
            Array * array = Array::WithType(type,(uint32_t)count);
            dynamic_cast<Object*>(array)->Read(hdr, ctx);
            ret = array;
        }else{
            Array * array = Array::WithCapacity();
            dynamic_cast<Object*>(array)->Read(hdr, ctx);
            ret = array;
        }
    }else if(hdr.GetFlag() == FLAG_MAP){
        Map * obj = new Map();
        dynamic_cast<Object*>(obj)->Read(hdr, ctx);
        ret = obj;
    }else if(hdr.GetFlag() == FLAG_OBJECT){
        Native * obj = new Native();
        dynamic_cast<Object*>(obj)->Read(hdr, ctx);
        ret = obj;
    }
    return ret;
}

Native::Native(){
    data = NULL;
}

Native::~Native(){
    if(data != NULL){
        free(data);
        data = NULL;
    }
}

Native * Native::Nil(){
    Native * ret = new Native();
    ret->header.SetType(TYPE_NIL);
    return ret;
}

Native * Native::Boolean (bool value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_BOOLEAN);
    ret->data = malloc(sizeof(uint8_t));
    *((uint8_t*)ret->data) = (uint8_t)value;
    return ret;
}

Native * Native::Byte (uint8_t value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_BYTE);
    ret->data = malloc(sizeof(uint8_t));
    *((uint8_t*)ret->data) = value;
    return ret;
}

Native * Native::Integer (int32_t value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_INT);
    ret->data = malloc(sizeof(int32_t));
    *((int32_t*)ret->data) = value;
    return ret;
}

Native * Native::Long (int64_t value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_LONG);
    ret->data = malloc(sizeof(int64_t));
    *((int64_t*)ret->data) = value;
    return ret;
}

Native * Native::Float (float value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_FLOAT);
    ret->data = malloc(sizeof(float));
    *((float*)ret->data) = value;
    return ret;
}

Native * Native::Double (double value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_DOUBLE);
    ret->data = malloc(sizeof(double));
    *((double*)ret->data) = value;
    return ret;
}

Native * Native::String (const std::string & value){
    Native * ret = new Native();
    ret->header.SetType(TYPE_STRING);
    ret->data = new std::string;
    *((std::string*)ret->data) = value;
    return ret;
}

bool Native::IsNil(){
    return GetType() == TYPE_NIL;
}

bool Native::GetBoolean(){
    if(GetType() == TYPE_BOOLEAN && *((uint8_t*)data) == 1)
        return true;
    return false;
}

uint8_t Native::GetByte(){
    if(GetType() == TYPE_BYTE)
        return *((uint8_t*)data);
    return 0;
}

int32_t Native::GetInt(){
    if(GetType() == TYPE_INT)
        return *((int32_t*)data);
    return 0;
}

int64_t Native::GetLong(){
    if(GetType() == TYPE_LONG)
        return *((int64_t*)data);
    return 0;
}

float Native::GetFloat(){
    if(GetType() == TYPE_FLOAT)
        return *((float*)data);
    return 0.0f;
}

double Native::GetDouble(){
    if(GetType() == TYPE_DOUBLE)
        return *((double*)data);
    return 0.0;
}

std::string Native::GetString(){
    if(GetType() == TYPE_STRING)
        return std::string((char*)data);//*((std::string*)data);
    return "";
}

void Native::Read(const Header & hdr,Context * ctx){
    header = hdr;
    
    if(GetType() == TYPE_NIL)
        return;
    
    int32_t len = (int32_t)header.GetLength();
    
    void * tmp = NULL;
    
    if(GetType() == TYPE_STRING)
        tmp = calloc(len + 1,1); // \0
    else
        tmp = calloc(len,1);
    
    if(ctx->Read(len, (uint8_t*)tmp) != len)
        throw EXCEPTION_READ_FAILURE;
    
    data = tmp;
}

/*
 void Native::Write(Context * ctx){
 header.Write(ctx);
 if(GetType()!=TYPE_NIL && GetLength()>0 && data != NULL)
 if(ctx->Write((int32_t)GetLength(), (uint8_t*)data)!= GetLength())
 throw EXCEPTION_WRITE_FAILURE;
 }
 */

Native * Native::WithData(NativeType type, void * data, int length){
    Native * ret = new Native();
    ret->header.SetType(type);
    
    if(type == TYPE_STRING)
        ret->data = calloc(1,length+1);
    else
        ret->data = calloc(1,ret->GetLength());
    
    memcpy(ret->data,data,length);
    
    return ret;
}

Array::~Array(){
    
}

NativeArray::NativeArray(NativeType type, uint32_t capacity){
    header.SetType(type);
    header.SetFlag(FLAG_ARRAY);
    this->nativeLen = (uint32_t)Object::SizeForNative(type);
    this->capacity = capacity;
}

NativeArray::~NativeArray(){
    if(data != NULL)
        delete (uint8_t*)data;
    data = NULL;
}

uint32_t NativeArray::GetSize(){
    return size;
}

uint32_t NativeArray::GetCapacity(){
    return 0;
}

Object * NativeArray::GetObject (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    
    int64_t offset = nativeLen * i;
    
    return Native::WithData(GetType(), (uint8_t*)data+offset, nativeLen);
}

Array * NativeArray::GetArray (int i){
    return NULL;
}

Map * NativeArray::GetMap (int i){
    return NULL;
}

bool NativeArray::IsNil (int i){
    return false;
}

bool NativeArray::GetBoolean (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_BOOLEAN){
        return ((uint8_t*)data)[i] == 1;
    }
    throw EXCEPTION_WRONG_TYPE;
}

uint8_t NativeArray::GetByte (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_BYTE){
        return ((uint8_t*)data)[i];
    }
    throw EXCEPTION_WRONG_TYPE;
}

int32_t NativeArray::GetInteger (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_INT){
        return ((int*)data)[i];
    }
    throw EXCEPTION_WRONG_TYPE;
}

int64_t NativeArray::GetLong (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_LONG){
        return ((int64_t*)data)[i];
    }
    throw EXCEPTION_WRONG_TYPE;
}

float NativeArray::GetFloat (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_FLOAT){
        return ((float*)data)[i];
    }
    throw EXCEPTION_WRONG_TYPE;
}

double NativeArray::GetDouble (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_DOUBLE){
        return ((double*)data)[i];
    }
    throw EXCEPTION_WRONG_TYPE;
}

std::string NativeArray::GetString (int i){
    if(i < 0 && i >= size)
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    if(GetType() == TYPE_STRING){
        char * str = ((char**)data)[i];
        return std::string(str);
    }
    throw EXCEPTION_WRONG_TYPE;
}

void NativeArray::Read(const Header & hdr,Context * ctx){
    header = hdr;
    int64_t len = header.GetLength();
    
    data = calloc(1,len);
    if(ctx->Read((int32_t)len, (uint8_t*)data) != len)
        throw EXCEPTION_READ_FAILURE;
    
    size = (int32_t)(len / nativeLen);
}


Array * Array::WithCapacity(uint32_t capacity){
    return new ObjectArray(capacity);
}

Array * Array::WithType(NativeType type, uint32_t capacity){
    return new NativeArray(type,capacity);
}

ObjectArray::ObjectArray(){
    
}

ObjectArray::ObjectArray(int capacity){
    array.reserve(capacity);
}

ObjectArray::~ObjectArray(){
    for(size_t i = 0;i<array.size();i++){
        delete array[i];
    }
    array.clear();
}

uint32_t ObjectArray::GetSize(){
    return (uint32_t)array.size();
}

uint32_t ObjectArray::GetCapacity(){
    return (uint32_t)array.capacity();
}

Object * ObjectArray::GetObject (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    return obj;
}

Array * ObjectArray::GetArray (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetFlag() == FLAG_ARRAY)
        return dynamic_cast<Array*>(obj);
    throw EXCEPTION_WRONG_TYPE;
}

Map * ObjectArray::GetMap (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetFlag() == FLAG_MAP)
        return dynamic_cast<Map*>(obj);
    throw EXCEPTION_WRONG_TYPE;
}

bool ObjectArray::IsNil (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    return obj->GetType() == TYPE_NIL;
}

bool ObjectArray::GetBoolean (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_BOOLEAN)
        return dynamic_cast<Native*>(obj)->GetBoolean();
    throw EXCEPTION_WRONG_TYPE;
}

uint8_t ObjectArray::GetByte (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_BYTE)
        return dynamic_cast<Native*>(obj)->GetByte();
    throw EXCEPTION_WRONG_TYPE;
}

int32_t ObjectArray::GetInteger (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_INT)
        return dynamic_cast<Native*>(obj)->GetInt();
    throw EXCEPTION_WRONG_TYPE;
}

int64_t ObjectArray::GetLong (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_LONG)
        return dynamic_cast<Native*>(obj)->GetLong();
    throw EXCEPTION_WRONG_TYPE;
}

float ObjectArray::GetFloat (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_FLOAT)
        return dynamic_cast<Native*>(obj)->GetFloat();
    throw EXCEPTION_WRONG_TYPE;
}

double ObjectArray::GetDouble (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_DOUBLE)
        return dynamic_cast<Native*>(obj)->GetDouble();
    throw EXCEPTION_WRONG_TYPE;
}

std::string ObjectArray::GetString (int i){
    if(i<0 || i >= array.size())
        throw EXCEPTION_ARRAY_OUT_OF_BOUNDS;
    Object * obj = array[i];
    if (obj == NULL)
        throw EXCEPTION_ARRAY_NULL_ENTRY;
    if(obj->GetType() == TYPE_STRING)
        return dynamic_cast<Native*>(obj)->GetString();
    throw EXCEPTION_WRONG_TYPE;
}

void ObjectArray::Read(const Header & hdr,Context * ctx){
    header = hdr;
    uint64_t ref = ctx->Tell();
    while( (ctx->Tell() - ref) < header.GetLength()){
        Object * obj = Object::Parse(ctx);
        array.push_back(obj);
    }
    
}

Map::Map(){
    
}

Map::~Map(){
    for(std::map<std::string,Object*>::iterator i = map.begin();i!=map.end();i++){
        Object * obj = i->second;
        delete obj;
    }
    map.clear();
}

uint32_t Map::GetSize(){
    return (uint32_t)map.size();
}

std::vector<std::string> Map::GetKeys(){
    std::vector<std::string> ret;
    ret.reserve(map.size());
    for(std::map<std::string,Object*>::iterator i = map.begin();i!=map.end();i++){
        ret.push_back(i->first);
    }
    return ret;
}

bool Map::HasKey (const std::string & key){
    return map.find(key)!=map.end();
}

Object * Map::GetObject (const std::string & key){
    std::map<std::string,Object*>::iterator i = map.find(key);
    if(i != map.end())
        return i->second;
    return NULL;
}

Array * Map::GetArray (const std::string & key){
    std::map<std::string,Object*>::iterator i = map.find(key);
    if(i != map.end()){
        if( i->second->GetFlag() == FLAG_ARRAY)
            return dynamic_cast<Array*>(i->second);
    }
    return NULL;
}

Map * Map::GetMap (const std::string & key){
    std::map<std::string,Object*>::iterator i = map.find(key);
    if(i != map.end()){
        if( i->second->GetFlag() == FLAG_MAP)
            return dynamic_cast<Map*>(i->second);
    }
    return NULL;
}

bool Map::IsNil (const std::string & key){
    Object * obj = GetObject(key);
    return obj!=NULL && obj->GetType() == TYPE_NIL;
}

bool Map::GetBoolean (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_BOOLEAN)
        return dynamic_cast<Native*>(obj)->GetBoolean();
    throw EXCEPTION_WRONG_TYPE;
}

uint8_t Map::GetByte (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_BYTE)
        return dynamic_cast<Native*>(obj)->GetByte();
    throw EXCEPTION_WRONG_TYPE;
}

int32_t Map::GetInteger (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_INT)
        return dynamic_cast<Native*>(obj)->GetInt();
    throw EXCEPTION_WRONG_TYPE;
}

int64_t Map::GetLong (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_LONG)
        return dynamic_cast<Native*>(obj)->GetLong();
    throw EXCEPTION_WRONG_TYPE;
}

float Map::GetFloat (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_FLOAT)
        return dynamic_cast<Native*>(obj)->GetFloat();
    throw EXCEPTION_WRONG_TYPE;
}

double Map::GetDouble (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_DOUBLE)
        return dynamic_cast<Native*>(obj)->GetFloat();
    throw EXCEPTION_WRONG_TYPE;
}

std::string Map::GetString (const std::string & key){
    Object * obj = GetObject(key);
    if(obj!=NULL && obj->GetType() == TYPE_STRING)
        return dynamic_cast<Native*>(obj)->GetString();
    throw EXCEPTION_WRONG_TYPE;
}

void Map::Read(const Header & hdr,Context * ctx){
    header = hdr;
    uint64_t ref = ctx->Tell();
    while( (ctx->Tell() - ref) < header.GetLength()){
        
        Header kh;
        kh.Read(ctx);
        
        if(kh.GetType() == TYPE_STRING){
            Native * native = new Native();
            dynamic_cast<Object*>(native)->Read(kh,ctx);
            
            if(native->GetString().size()>0){
                Object * obj = Object::Parse(ctx);
                map.insert(std::pair<std::string,Object*>(native->GetString(),obj));
                delete native;
            }
        }else{
            throw EXCEPTION_WRONG_TYPE;
        }
    }
}

std::string Object::ToJSON(){
    return std::string();
}

_BXON_END


