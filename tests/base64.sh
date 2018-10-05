#!/bin/env sh

# This tests expects that there is a reference implementation of base64 in the
# PATH

REAL="/tmp/mybase64/"
REF="/tmp/refbase64/"

OK="\e[32;1mOK\e[0m"
ERROR="\e[31;1mERROR\e[0m"

if ! mkdir $REAL $REF ;then
	echo "Couldn't create folders: ${REAL} ${REF}"
	exit
fi
	
for i in /etc/*;do
	if [ -r "$i" ] && [ -f "$i" ];then
		file=$(basename $i)
		base64 $i >${REF}${file}
		../src/base64 $i >${OUR}${file}
		echo -n "Testing ${file}.. "
		if diff ${OUR}${file} ${REF}${file} 2>&1 >/dev/null;then
			echo -e $OK
		else
			echo -e $ERROR
		fi
	fi
done

rm -r $REAL $REF
