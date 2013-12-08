#!/usr/bin/env python
# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This utility computes and fills Exynos ROM checksum (for BL1 or BL2).
(Algorithm from U-Boot: tools/mkexynosspl.c)

Input: IN OUT

Output:

  Checksum header added to IN and written to OUT.
  Header: uint32_t size, checksum, reserved[2].
"""

import struct
import sys

def main(argv):
  if len(argv) != 3:
    exit('usage: %s IN OUT' % argv[0])

  in_name, out_name = argv[1:3]
  header_format = "<IIII"
  with open(in_name, "rb") as in_file, open(out_name, "wb") as out_file:
    data = in_file.read()
    header = struct.pack(header_format,
                         struct.calcsize(header_format) + len(data),
                         sum(map(ord, data)),
                         0, 0)
    out_file.write(header + data)


if __name__ == '__main__':
  main(sys.argv)
