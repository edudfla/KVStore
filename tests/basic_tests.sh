#!/bin/bash
test_binary=../x64/Debug/KVStore.exe
declare tests=(
	"a b c"
	"a(1);b(1);c(1);"
	"a a a"
	"a(3);"
	"a ab abc"
	"a(1)b(1)c(1);"
)
basic_test_1=$($test_binary a b c)
declare -i i=0
declare -i j=${#tests[@]}
while ((i < j))
do
	result=$(eval "$test_binary ${tests[i]}")
	((++i))
	[ "$result" == "${tests[i]}" ] || echo "FAIL ${tests[i-1]} ${tests[i]} $result"
	((++i))
done
