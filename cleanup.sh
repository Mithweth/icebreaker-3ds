#!/bin/bash -eu

TYPE=$1
APPLY=${2:-}

TMPFILE=$(mktemp)
grep -H "^${TYPE}.*(" *.c *.h | while IFS=: read -r fil ftn; do
	f1=$(cut -d. -f1 <<< "$fil")
	fname=$(sed "s/^$TYPE \([^(]*\)(.*$/\1/1" <<< "$ftn")
	if grep -q "^${f1}_" <<< "$fname"; then
		continue
	fi
	if grep -qw "$f1" <<< "$fname"; then
		continue
	fi
	newftn=
	if grep -q "$f1" <<< "$fname"; then
		newftn="${f1}_$(sed "s/$f1//1" <<< "$fname")"
	else
		newftn="${f1}_$fname"
	fi
	echo "$fname:$newftn" >> "$TMPFILE"
done
cat "$TMPFILE"

if [ "$APPLY" != "-y" ]; then
	rm -f "$TMPFILE"	
	exit 0
fi

while IFS=: read -r ftn newftn; do
	sed -i "s/\<$ftn\>/$newftn/g" *.c *.h
done < "$TMPFILE"

rm -f "$TMPFILE"
