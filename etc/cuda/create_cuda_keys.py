#! /usr/bin/env python

#
# Copyright (C) 2010
# Sascha Hunold <sascha@icsi.berkeley.edu>
#

import sys
import re
from launchpadbugs.lphelper import sort

START_KEY_ID=1
current_key_id=START_KEY_ID

arg_type_set = set()

def parse_function(func):
    ret_type  = None
    func_name = None
    args      = []  # list of tupels [ (type, name) ]
    
    #print func,
    
    # parsing arguments
    m = re.match(".*\((.*)\).*", func)
    if not m:
        print >> sys.stderr, "Cannot parse arguments in line:", func
    else:
        arg_str = m.group(1)
        arg_arr = arg_str.split(",")
        for i in xrange(0, len(arg_arr)):
            arg_arr[i] = arg_arr[i].strip()
        #print arg_arr
        for arg in arg_arr:
            if arg == "void":
                    args.append( ["void", None])
            else:
                arg = arg[::-1] # reverse string
                m = re.match("([\w]+)([\*\s].*)", arg)
                if not m:
                    print  >> sys.stderr,  "Cannot parse argument:", arg 
                else:
                    arg_name = m.group(1)[::-1]
                    arg_name = arg_name.strip()
                    #print arg_name
                    arg_type = m.group(2)[::-1]
                    arg_type = arg_type.strip()
                    args.append( [arg_type, arg_name])
    
    # parsing func name and return type
    m = re.match("(.*)\(.*\).*", func)
    if not m:
        print >> sys.stderr, "Cannot parse function name in line:", func
    else:
        #print "name", m.group(1)
        func_name_str = m.group(1)[::-1]
        func_name_str = func_name_str.strip() # remove whitespace before and behind
        m = re.match("([\S]+)(.*)", func_name_str) # anything until I see a whitespace
        # print func_name_str
        if not m:
            print >> sys.stderr, "Cannot detect function name in line:", func
        else:
            func_name = m.group(1)[::-1]
            ret_type  = m.group(2)[::-1]
            ret_type  = ret_type.strip()
        
    
    return { "ret_type"  : ret_type,
             "func_name" : func_name,
             "args"      : args }
    

def convert_to_ipmkey(line, func_hash):
    global current_key_id
    key_str = ""
    
    
    key_str += str(current_key_id) + "|"
    current_key_id += 1
    
    key_id_name = func_hash["func_name"]
    key_id_name = key_id_name.replace("cuda", "cuda_")
    key_id_name = key_id_name.upper()
    key_str += key_id_name + "_ID|"
    
    # add prototype
    line = line.strip()
    line = re.sub("\s+", " ", line)
    line = re.sub("\s\(", '(', line) #remove whitespace before left parenthesis
    key_str += line 
    
    key_str += "||CS_LOCAL,BYTES_NONE,RANK_NONE,DATA_NONE,COMM_NONE"
    
    return key_str

def extract_arg_types(arg_type_set, func_hash):
    
    for arg_tupel in func_hash["args"]:
        arg_type_set.add(arg_tupel[0])


def sort_by_length(s1, s2):
    return len(s1)-len(s2) 
    

if __name__ == "__main__":

    API_FILE="cuda_api.txt"
    
    fh = open(API_FILE)
    for line in fh:
        if re.match("\s*#.*", line):
            # remove comments
            continue
        elif len(line.strip()) <= 0:
            # remove empty line
            continue
        else:
            func_hash = parse_function(line)
#            print >> sys.stderr, "l:", line,
#            print >> sys.stderr, "ret :", func_hash["ret_type"]
#            print >> sys.stderr, "func:", func_hash["func_name"]
#            print >> sys.stderr, "args:",
#            for arg_tupel in func_hash["args"]:
#                print arg_tupel[0],
#                if arg_tupel != "void":
#                    print >> sys.stderr, arg_tupel[1],
#                print >> sys.stderr, ",",
#            print >> sys.stderr, ""
            ipm_key = convert_to_ipmkey(line, func_hash)
            sys.stdout.write(ipm_key + "\n") 
            extract_arg_types(arg_type_set, func_hash)
            
    fh.close()
    
    arg_type_list = list(arg_type_set)
    arg_type_list.sort(cmp=sort_by_length, reverse=True)
    
    for arg_type in arg_type_list:
        arg_type = re.sub("\*", "\\*", arg_type)
        print "$call{$id}{car} =~ s/%s//g;" % ( arg_type )
        
    
    
    
    