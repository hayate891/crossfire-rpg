#!/bin/sh
# Make default key bindings into header file
if [ -f def-keys.h ] ; then
  rm def-keys.h
fi
echo "const char *const def_keys[] = {" >def-keys.h
cat "$1/def-keys" | sed 's/^/"/' | sed 's/$/\\n",/' >>def-keys.h
echo "};" >>def-keys.h
