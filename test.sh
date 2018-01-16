#! /bin/bash -x

test() {
    expected="$1"
    expr="$2"

    echo "$expr" | ./konoha > tmp/out.s
    if [ $? != 0 ]; then
	exit -1
    fi
    gcc tmp/out.s driver.c -o tmp/a.out
    if [ $? != 0 ]; then
	exit -1
    fi
    res=`./tmp/a.out`
    if [ "x$res" != "x$expected" ]; then
	echo "Test failed: expected $expected, but got $res"
	exit -1
    fi
}

test "0" "0"
test "42" "42"
test "100" "100"
