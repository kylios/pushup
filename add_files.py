#!/usr/bin/python

# Adds a .c .h pair of empty files in the include/ and src/ directories
# respectively with the given name.  Provided paths should be respective
# to src/ or include/ such that invoking like so:
#
#       add_files.py lib/kernel/rb_tree 
#
# should add src/lib/kernel/rb_tree.c and include/lib/kernel/rb_tree.h

import sys
import os
import string
import datetime

if len (sys.argv) < 1:
    print("Awww come on, supply an argument so I can create some files!\n")
else:
    # Read in the program header so we can automatically insert it into
    # the top of our files.
    h = open ("program_header.txt", 'r')
    header = h.read ()
    h.close ()

    # Get the date and format it right
    now = datetime.datetime.now ()
    datestr = now.strftime ("%Y-%m-%d %H:%M")

    # First stab at constructing the file names
    src_filename = sys.argv[1] + ".c"
    header_filename = sys.argv[1] + ".h"

    # Generate the #include "header.h" for the source file
    src_include_str = ""
    #src_include_str += "#include \"type.h\"\n"
    #src_include_str += "#include \"common.h\"\n"
    #src_include_str += "#include \"debug.h\"\n"
    src_include_str += "#include \"" + header_filename + "\"\n\n"

    header_include_str = ""
    header_include_str += "#include \"type.h\"\n"
    header_include_str += "#include \"common.h\"\n"

    # Generate the series of #ifndef and #defines for the header file
    header_define_str = string.replace (header_filename, os.sep, "_")
    header_define_str = string.replace (header_define_str, ".", "_")
    header_define_str = header_define_str.upper ()
    header_define_str = "#ifndef " + header_define_str + \
                         "\n#define " + header_define_str + \
                         "\n\n" + \
                         header_include_str + \
                         "\n\n\n\n#endif  // " + header_define_str

    # Finish constructing the actual filenames
    src_filename = "src" + os.sep + src_filename
    header_filename = "include" + os.sep + header_filename

    # Construct the contents of each file
    src_header = string.replace (header, "${file_name}", src_filename)
    src_header = string.replace (src_header, "${date}", datestr)
    src_header += src_include_str + "\n"
    header_header = string.replace (header, "${file_name}", header_filename)
    header_header = string.replace (header_header, "${date}", datestr)
    header_header += header_define_str

    # Now create and write the new files
    f = open (src_filename, 'w')
    f.write (src_header)
    f.close ()
    f = open (header_filename, 'w')
    f.write (header_header)
    f.close ()

