#! /bin/bash -x

konoha=./konoha

compile() {
    echo "$1" | "$konoha" > tmp/out.s
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
    : test "expected $expected, expr $expr"

    compile "$expr"
    res=`./tmp/a.out`
    if [ "x$res" != "x$expected" ]; then
	echo "Test failed: expected $expected, but got $res"
	exit -1
    fi
}

test_ast() {
    expected="$1"
    expr="$2"
    : test_ast "expected $expected, expr $expr"

    res=`echo "$expr" | "$konoha" -a`
    if [ "x$res" != "x$expected" ]; then
	echo "Test failed: expected $expected, but got $res"
	exit -1
    fi
}

test_ast "0" "0"
test_ast "42" "42"
test_ast "100" "100"
test "0" "0"
test "42" "42"
test "100" "100"

test_ast "(add 0 0)" "0+0"
test_ast "(add 1 2)" "1+2"
test_ast "(add 100 200)" "100 +     200"
test_ast "(add 42 42)" "42
+
42"
test "0" "0+0"
test "3" "1+2"
test "300" "100 +     200"
test "84" "42
+
42"

test_ast "(sub 0 0)" "0-0"
test_ast "(sub 2 1)" "2-1"
test_ast "(sub 1 2)" "1-2"
test "0" "0-0"
test "1" "2-1"
test "-1" "1-2"

test_ast "(imul 0 0)" "0*0"
test_ast "(imul 1 2)" "1*2"
test_ast "(imul 33 3)" "33*3"
test "0" "0*0"
test "2" "1*2"
test "99" "33*3"

test_ast "(add (add 1 2) 3)" "1+2+3"
test_ast "(imul (imul 2 3) 4)" "2*3*4"
test "6" "1+2+3"
test "24" "2*3*4"

test_ast "(add 1 (imul 2 3))" "1+2*3"
test_ast "(add (imul 3 4) 5)" "3*4+5"
test_ast "(add (add 1 (imul 2 3)) 4)" "1+2*3+4"
test "7" "1+2*3"
test "17" "3*4+5"
test "11" "1+2*3+4"
