#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import re

data = '''
DP 54:25:41 N 019:53:29 E
DP 53:34:59 N 019:34:00 E
DP 52:38:59 N 019:07:59 E
DP 52:40:00 N 018:19:59 E
DP 53:13:59 N 016:41:59 E
DP 52:34:59 N 014:37:59 E
DP 52:34:59 N 014:37:59 E
DP 53:13:59 N 016:41:59 E
DP 52:40:00 N 018:19:59 E
DP 52:38:59 N 019:07:59 E
DP 53:34:59 N 019:34:00 E
DP 54:25:41 N 019:53:29 E
'''

danzig='''
DP 52:35:00 N 014:38:00 E
DP 54:25:42 N 019:53:29 E
DP 53:35:00 N 019:34:00 E
DP 52:39:00 N 019:08:00 E
DP 52:40:00 N 018:20:00 E
DP 53:14:00 N 016:42:00 E
DP 52:35:00 N 014:38:00 E
'''

if __name__ == "__main__" :

    lines = re.split("[\n]", data)

    out = ""
    out1 = ""

    for line in lines:
        if(line == ''):
            continue

        elements = line.split()

        late = elements[1].split(':')
        lone = elements[3].split(':')

        lat = ((((float(late[2]) / 60.0) + float(late[1])) / 60.0) + float(late[0]))
        lon = ((((float(lone[2]) / 60.0) + float(lone[1])) / 60.0) + float(lone[0]))

        if(out == "") :
            out = "%f %f" % (lon, lat)
        else:
            out += ", %f %0f" % (lon, lat)

        if(out1 == "") :
            out1 = "%f %f" % (lat, lon)
        else:
            out1 += ", %f %0f" % (lat, lon)
            
    print out
    print
    print out1

