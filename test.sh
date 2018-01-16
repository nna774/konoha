#! /bin/sh -x

./konoha > tmp/out.s
if [ ! $? ]; then
    exit -1
fi
gcc tmp/out.s -o tmp/a.out
./tmp/a.out
if [ $? != 42 ]; then
    exit -1
fi
