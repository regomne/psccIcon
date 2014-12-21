import re
import struct

def GetStruct(ss):
    s=[]
    pos=0
    for i in range(40):
        s.append(int(ss[pos:pos+8],16))
        pos+=8
    return s

pat=re.compile(r'[^:]+: "([ABCDEF0123456789]*)".*')

ls=open('IconResources.idx','rb').read().decode().split('\r\n')

for l in ls:
    mo=pat.match(l)
    if mo:
        s=GetStruct(mo.group(1))
        for i in range(len(s)):
            if ((i%4==2) or (i%4==3)): #and ((i%20!=2) and (i%20!=3)):
                if s[i]!=0:
                    print(l)
                    break
        
