#! /bin/bash -x

compile() {
    echo "$1" | ./konoha > tmp/out.s
    if [ $? != 0 ]; then
	echo "compilation fail"
	exit -1
    fi
    gcc tmp/out.s driver.c -o tmp/a.out
    if [ $? != 0 ]; then
	echo "gcc fail"
	exit -1
    fi
}

test() {
    expected="$1"
    expr="$2"

    compile "$expr"
    res=`./tmp/a.out`
    if [ "x$res" != "x$expected" ]; then
	echo "Test failed: expected $expected, but got $res"
	exit -1
    fi
}

test "0" "0"
test "42" "42"
test "100" "100"
