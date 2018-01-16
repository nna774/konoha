#! /bin/sh -x

./konoha > tmp/out.s
if [ $? != 0 ]; then
    exit -1
fi
gcc tmp/out.s -o tmp/a.out
if [ $? != 0 ]; then
    exit -1
fi
./tmp/a.out
if [ $? != 42 ]; then
    exit -1
fi
