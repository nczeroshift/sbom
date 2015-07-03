import struct, time, sys, os, subprocess, math

BXON_NIL        = 0
BXON_STRING     = 1
BXON_BOOLEAN    = 2
BXON_INT        = 3
BXON_LONG       = 4
BXON_FLOAT      = 5
BXON_DOUBLE     = 6
BXON_BYTE       = 7

BXON_LENGTH_8   = 0x00
BXON_LENGTH_16  = 0x10
BXON_LENGTH_32  = 0x20
BXON_LENGTH_64  = 0x30

BXON_OBJECT     = 0x00
BXON_ARRAY      = 0x40
BXON_MAP        = 0x80

class bxon_context(object):
    file = None
    def __init__(self,f):
        self.file = f

    def tell(self):
        return self.file.tell()

    def seek(self,p):
        self.file.seek(p,0)

    def write(self,p,v):
        self.file.write(struct.pack(p,v))

    def __del__(self):
        self.file.close()

    def lengthForNative(self,t):
        if(t == BXON_BOOLEAN):
            return 1
        elif(t == BXON_INT):
            return 4 
        elif(t == BXON_LONG):
            return 8
        elif(t == BXON_FLOAT):
            return 4
        elif(t == BXON_DOUBLE):
            return 8
        elif(t == BXON_BYTE):
            return 1
            
    def writeNative(self,t,val):
        if(t == BXON_BOOLEAN):
            self.write("<B",val) 
        elif(t == BXON_INT):
            self.write("<i",val) 
        elif(t == BXON_LONG):
            self.write("<q",val) 
        elif(t == BXON_FLOAT):
            self.write("<f",val) 
            #print("float")
        elif(t == BXON_DOUBLE):
            self.write("<d",val) 
        elif(t == BXON_BYTE):
            self.write("<B",val) 
            
class bxon_native(object):
    type = None
    value = None

    def __init__(self, t, v):
        self.type = t
        self.value = v

    def write(self,ctx):
        if(self.type == BXON_NIL):
            ctx.write("<B",BXON_NIL)
        elif(self.type == BXON_STRING):
            ctx.write("<B",BXON_STRING | BXON_LENGTH_32)
            data = self.value.encode("utf-8");
            ctx.write("<i",len(data))
            for c in data:
                ctx.file.write(struct.pack("<s",c))   
        elif(self.type == BXON_BOOLEAN):
            ctx.write("<B",BXON_BOOLEAN)
            ctx.write("<B",self.value) 
        elif(self.type == BXON_INT):
            ctx.write("<B",BXON_INT)
            ctx.write("<i",self.value)      
        elif(self.type == BXON_LONG):
            ctx.write("<B",BXON_LONG)
            ctx.write("<q",self.value)
        elif(self.type == BXON_FLOAT):
            ctx.write("<B",BXON_FLOAT)
            ctx.write("<f",self.value)  
        elif(self.type == BXON_DOUBLE):
            ctx.write("<B",BXON_DOUBLE)
            ctx.write("<d",self.value)  
        elif(self.type == BXON_BYTE):
            ctx.write("<B",BXON_BYTE)
            ctx.write("<B",self.value)
    
class bxon_map(object):
    parent = None
    context = None
    startPos = None
    endPos = None
    #map = {}

    def __init__(self,ctx=None):
        self.parent = None
        self.context = ctx
        self.startPos = None
        self.endPos = None
        #self.map = {}

    def write(self,p=None):
        if(p!=None):
            self.parent = p;
            self.context = p.context;
        
        if(self.startPos == None):
            self.context.write("<B",BXON_MAP|BXON_LENGTH_64)
            self.context.write("<q",0)
            self.endPos = self.startPos = self.context.tell()

    def _update(self):
        self.endPos = self.context.tell()
        lenght = self.endPos - self.startPos;
        self.context.seek(self.startPos-8);
        self.context.write("<q",lenght)
        self.context.seek(self.endPos);
        
        if(self.parent != None ):
            self.parent._update();
            
    def put(self, key, vObj):
        self.write()
        kObj = bxon_native(BXON_STRING,key);
        kObj.write(self.context)
        if type(vObj) is bxon_native:
            vObj.write(self.context)
        else:
            vObj.write(self)
        #self.map[key] = vObj;
        self._update();
        return vObj;

class bxon_array(object):
    parent = None
    context = None
    startPos = None
    endPos = None
    nativeType = BXON_NIL
    nativeCount = 0
    nativeIndex = 0
    
    #array = []
    def __init__(self,ctx=None,nType=BXON_NIL,nCount=0):
        self.parent = None
        self.context = ctx
        self.startPos = None
        self.endPos = None
        self.nativeType = BXON_NIL;
        if(nType!=BXON_NIL and nType!=BXON_STRING):
            self.nativeType = nType
            self.nativeCount = nCount
            self.nativeIndex = 0

    def write(self,p=None):
    	if(p!=None):
            self.parent = p;
            self.context = p.context;
    	    
        if(self.startPos == None):
            self.context.write("<B",BXON_ARRAY|BXON_LENGTH_64|self.nativeType)
            self.context.write("<q",0)
            self.startPos = self.context.tell()
            if(self.nativeType != BXON_NIL):
                totalBytes = self.nativeCount * self.context.lengthForNative(self.nativeType)
                #print("total size",totalBytes)
                #for i in range (totalBytes):
                #    self.context.write("<B",0)
                  
    def _update(self):
    	
    	"""
        self.endPos = self.context.tell()
        lenght = self.endPos - self.startPos;
        self.context.seek(self.startPos-8);
        self.context.write("<q",lenght)
        #print(lenght)
        self.context.seek(self.endPos);
        if(self.parent != None ):
            self.parent._update();
        """
    def push(self, obj):
        self.write()
        if self.nativeType != BXON_NIL:    
            pos = self.startPos + self.nativeIndex * self.context.lengthForNative(self.nativeType)
            if pos!=self.context.tell():
                self.context.seek(pos)
                #print("seek to",pos)
            
            if type(obj) == bxon_native:
                self.context.writeNative(self.nativeType,obj.value)
            else:
                self.context.writeNative(self.nativeType,obj)
            self.nativeIndex+=1
            
            #self.context.writeNative(BXON_FLOAT,obj.value)
        else:
            self.write()
            if type(obj) is bxon_native:
                obj.write(self.context)
            else:
                obj.write(self)
            #self.array.append(obj);
        self._update()
        return obj

def test():
    start_time = time.time()
    
    f = open("../out2.bxon","wb")
    for i in range(30000):
    	f.write(struct.pack("<f",10000.0))
    f.close()
    elapsed_time = time.time() - start_time
    print(elapsed_time)
    
    
    
    start_time = time.time()
    f = open("../out.bxon","wb")
    ctx = bxon_context(f)

    root = bxon_map(ctx)

    #root.put("temp1", bxon_native(BXON_INT,10))
    #root.put("temp2", bxon_native(BXON_FLOAT,10.0))
    #root.put("temp3", bxon_native(BXON_DOUBLE,12.0))
    #root.put("temp4", bxon_native(BXON_STRING,"test string!"))

    array = root.put("array",bxon_array(nType=BXON_FLOAT,nCount = 30000))
    
    for i in range(10000):
    	array.push(bxon_native(BXON_FLOAT,i));
    	array.push(bxon_native(BXON_FLOAT,i*3));
    	array.push(bxon_native(BXON_FLOAT,i*6));
    	
    #array.push(bxon_native(BXON_FLOAT,2.0));
    #array.push(bxon_native(BXON_FLOAT,3.0));

    #root.put("bool_5", bxon_native(BXON_BOOLEAN,True))
    elapsed_time = time.time() - start_time
    print(elapsed_time)
    
if __name__ == "__main__":
    test()

