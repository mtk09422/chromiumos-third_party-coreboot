#!/bin/bash

# This script is a filter processing XML files.
#
# Processing is as follows:
#
# - get rid of DOS line ends;
# - place each opening tag on a separate line, with no leading spaces and the
#   first attribute definition following the tag name;
# - place each attribute definition on a separate line, starting with a space;
#
# usage:
#  xml_converter.sh '<unformatted-xml-file>' > '<formatted-xml-file>'
#
cat $1 | sed 's/></>\n</g;
s/\r$//;
s/^[\t ]\+//;
s/" /"\n /g;
s/^/ /;
s/^ </</;
/^\s*$/d;'
