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

test "0" "0+0"
test "3" "1+2"
test "300" "100 +     200"
test "84" "42
+
42"

test "0" "0-0"
test "1" "2-1"
test "-1" "1-2"

test "0" "0*0"
test "2" "1*2"
test "99" "33*3"

test "6" "1+2+3"
test "24" "2*3*4"

test "7" "1+2*3"
