#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import re

data = '''
DP 52:34:59 N 014:37:59 E
DP 53:13:59 N 016:41:59 E
DP 52:40:00 N 018:19:59 E
DP 52:38:59 N 019:07:59 E
DP 51:42:00 N 018:37:59 E
DP 51:13:00 N 018:34:00 E
DP 51:00:00 N 018:28:00 E
DP 50:04:00 N 018:01:59 E
DP 50:04:00 N 018:01:59 E
DP 51:00:00 N 018:28:00 E
DP 51:13:00 N 018:34:00 E
DP 51:42:00 N 018:37:59 E
DP 52:38:59 N 019:07:59 E
DP 52:40:00 N 018:19:59 E
DP 53:13:59 N 016:41:59 E
DP 52:34:59 N 014:37:59 E
'''

if __name__ == "__main__" :

    lines = re.split("[\n]", data)

    out = ""

    for line in lines:
        if(line == ''):
            continue

        elements = line.split()

        late = elements[1].split(':')
        lone = elements[3].split(':')

        lat = ((((float(late[2]) / 60.0) + float(late[1])) / 60.0) + float(late[0]))
        lon = ((((float(lone[2]) / 60.0) + float(lone[1])) / 60.0) + float(lone[0]))

        if(out == "") :
            out = "%0.12f %0.12f" % (lon, lat)
        else:
            out += ", %0.12f %0.12f" % (lon, lat)

    print out

