#!/usr/bin/env bash
file="$1"
line="$2"

start=$((line-5))
end=$((line+5))

nl -ba "$file" | awk -v s="$start" -v e="$end" -v l="$line" '
NR>=s && NR<=e {
  if (NR==l) print ">>> " $0;
  else print "    " $0
}'
