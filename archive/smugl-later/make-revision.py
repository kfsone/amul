#! /usr/bin/python
#
# Generate "release number" (revision.h) files.
#
# Using a ".rev" file to track current release numbers,
# the tool provides a mechanism for automatic increment
# of revision numbers.

# Defaults
incrementVersion = False
incrementRevision = False
outputFile = "revision.h"
inputFile = "project.rev"
minor = None
major = None
build = 0
debug = False
prefix = ""

from os import getenv
import sys

usage = "Usage: make-revision.py [-v | -r] [-i <input file>] [-o <output file>] [-p <prefix for #defines>] [minor [major]]"

# Parse arguments.
argNo = 1
while argNo < len(sys.argv) and sys.argv[argNo][0] == "-":
#{
    arg = sys.argv[argNo]
    argNo += 1

    if arg == "-v":
        incrementVersion = True
        continue
    elif arg == "-r":
        incrementRevision = True
        continue
    elif arg == "-d":
        debug = True
        continue

    if argNo >= len(sys.argv):
        raise Exception("Missing value after %s" % arg)
    value = sys.argv[argNo]
    argNo += 1

    if arg == "-p":
        prefix = value
        continue
    elif arg == "-i":
        inputFile = value
        continue
    elif arg == "-o":
        outputFile = value
    else:
        raise Exception("Unrecognized argument, %s" % arg)
#} end of arg parsing

# Read the min/maj version numbers.
if argNo < len(sys.argv):
#{
    minor = int(sys.argv[argNo])
    argNo += 1
    if argNo < len(sys.argv):
        major = int(sys.argv[argNo])
        argNo += 1
        if argNo < len(sys.argv):
            raise Exception("Too many arguments.\n%s" % usage)
#} end version numbers

if debug:
#{
    print("Ok:")
    print(" pre: %s" % (prefix if prefix else "<none>"))
    print(" inf: %s" % (inputFile))
    print(" out: %s" % (outputFile))
    print(" v++: %s" % ("yes" if incrementVersion else "no"))
    print(" r++: %s" % ("yes" if incrementRevision else "no"))
    print(" maj: %s" % ("None" if major == None else ("%u" % major)))
    print(" min: %s" % ("None" if minor == None else ("%u" % minor)))

try:
    f = open(inputFile, "r", 1)
    major = int(f.readline())
    minor = int(f.readline())
    build = int(f.readline())
    f.close()
except:
    pass

if major == None or minor == None:
    raise Exception("New revision file, Major and Minor versions required.")

build += 1
if incrementVersion:
    major += 1
    minor = 0
elif incrementRevision:
    minor += 1

from datetime import datetime
built = datetime.now().strftime("%a %d %b %Y %H:%M:%S")
builtDay = datetime.now().strftime("%Y/%m/%d")

try:
    f = open(outputFile, "w")
    f.write("#pragma once\n")
    f.write("#define %s_BUILD_VERSION  %u\n" % (prefix, major))
    f.write("#define %s_BUILD_REVISION %u\n" % (prefix, minor))
    f.write("#define %s_BUILD_BUILDNO  %04u\n" % (prefix, build))
    f.write("#define %s_BUILD_BUILT    \"v%u.%u.%04u (%s)\"\n" % (prefix, major, minor, build, built))
    f.write("#define %s_BUILD_DATE     \"%s\"\n" % (prefix, builtDay))
    f.close()
except:
    raise Exception("Unable to write out header file %s" % outputFile)

try:
    f = open(inputFile, "w", 1)
    f.write("%u\n%u\n%04u\n" % (major, minor, build))
    f.close()
except:
    raise Exception("Unable to access revision file %s" % inputFile)

print("v%u.%u.%04u %s" % (major, minor, build, built))
