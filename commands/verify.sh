#!/bin/bash
pattern="$1"
folder="$2"
set +x
grep $pattern $folder -r -n | sort -s > grep.out
fast-grep $pattern $folder -n | sort -s > fast-grep.out
diff fast-grep.out grep.out
set -x
