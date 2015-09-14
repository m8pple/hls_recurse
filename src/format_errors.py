# -*- coding: utf-8 -*-

import fileinput
import re

def format_type(line,indent):
    res=""
    skip=False
    for i in range(len(line)):
        c=line[i]
        if c=="<":
            nextOpen=line.find("<",i+1)
            nextClose=line.find(">",i+1)
            if nextClose < nextOpen:
                skip=True
                res=res+"<"
            else:
                res=res+"<\n"
                indent=indent+"  "
                res=res+indent
        elif c==",":
            res=res+","
            if not skip:
                res=res+"\n"
                res=res+indent
        elif c==">":
            if not skip:
                res=res+"\n"
                indent=indent[2::]
                res=res+indent
            else:
                skip=False
            res=res+">"
        else:
            res=res+c
    return res

for line in fileinput.input():
    line=re.sub("hls_recurse::mutual::","hlsm::", line)
    line=re.sub("hls_recurse::","hls::", line)
   
    s=lambda m: "  \n  "+format_type(m.group(0).strip("‘’"), "  ")
    pattern="‘[^']+’"
    line=re.sub(pattern, s, line)
   
    #s=lambda m: "  \n"+format_type(m.group(0).strip("'"), "  ")
    #pattern="'[^']+'"
    #line=re.sub(pattern, s, line)
    
    pattern=": note: .*$"
    line=re.sub(pattern, s, line)
    
    
    print(line)
