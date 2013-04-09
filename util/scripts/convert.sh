#!/bin/bash
#
# This file is part of the coreboot project.
#
# Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
#
# Use this file to convert intel microcode represented as an assembler source
# (with `dd <hex number>h' in each line) into a C source (each number
# represented as `0x<num>, ', four per line.
#
# convert.sh 'intel asm file' > 'coreboot C file'
#

awk '
BEGIN { n = 0 };
{ sub(";.*", ""); }
 /^dd / {
 sub ("^dd 0","dd ");
 sub ("h.*$","", $2);
 printf (" 0x%s,", $2);
 n = n + 1;
 if (n ==4) {
    printf ("\n");
    n = 0;
 }
}' $1
