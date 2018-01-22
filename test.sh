#! /bin/bash -x

konoha=./konoha

compile() {
    echo "$1" | "$konoha" > tmp/out.s
    if [ $? != 0 ]; then
	echo "compilation fail"
	exit -1
    fi
    "$CC" tmp/out.s driver.c -o tmp/a.out
    if [ $? != 0 ]; then
	echo "$CC fail"
	exit -1
    fi
}

test() {
    expected="$1"
    expr="$2"
    : test "expected $expected, expr $expr"

    compile "$expr"
    res=`./tmp/a.out`
    if [ $? != 0 ]; then
	echo "execution fail"
	exit -1
    fi
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
    if [ $? != 0 ]; then
	echo "execution fail"
	exit -1
    fi
    if [ "x$res" != "x$expected" ]; then
	echo "Test failed: expected $expected, but got $res"
	exit -1
    fi
}

test_ast "(do 0)" "{0;}"
test_ast "(do 42)" "{42;}"
test_ast "(do 100)" "{100;}"

test_ast "(do 0)" "{    0     ;}"

# test_ast "0;1" "{0;1;}" admitted

test_ast "(do (add 0 0))" "{0+0;}"
test_ast "(do (add 1 2))" "{1+2;}"
test_ast "(do (add 100 200))" "{100 +     200;}"
test_ast "(do (add 42 42))" "{42
+
42;}"

test_ast "(do (sub 0 0))" "{0-0;}"
test_ast "(do (sub 2 1))" "{2-1;}"
test_ast "(do (sub 1 2))" "{1-2;}"
test_ast "(do (sub (sub (sub 1 2) 3) 4))" "{1-2-3-4;}"

test_ast "(do (imul 0 0))" "{0*0;}"
test_ast "(do (imul 1 2))" "{1*2;}"
test_ast "(do (imul 33 3))" "{33*3;}"

test_ast "(do (add (add 1 2) 3))" "{1+2+3;}"
test_ast "(do (imul (imul 2 3) 4))" "{2*3*4;}"

test_ast "(do (add 1 (imul 2 3)))" "{1+2*3;}"
test_ast "(do (add (imul 3 4) 5))" "{3*4+5;}"
test_ast "(do (add (add 1 (imul 2 3)) 4))" "{1+2*3+4;}"

test_ast "(do (defvar a))" "{ int a; }"
test_ast "(do (defvar a)(eval a))" "{int a; a;}"
test_ast "(do (defvar a)(let a 0))" "{int a;a=0;}"
test_ast "(do (defvar a)(let a (add 1 2)))" "{int a; a=1+2;}"
test_ast "(do (defvar a)(defvar b)(let a 0)(let b 0))" "{int a; int b; a=0; b=0;}"
test_ast "(do (defvar a)(let a 0)(defvar b)(let b 0))" "{int a; a=0; int b; b=0;}"

test_ast "(do (defvar a)(defvar b)(let a 1)(let b 2)(add (add (imul (eval a) (eval b)) (imul (eval a) 3)) (imul (eval b) 2)))" "{int a; int b;a=1;b=2;a*b+a*3+b*2;}"
test_ast "(do (defvar a)(let a 1)(let a (add (eval a) 2))(eval a))" "{int a;a = 1; a = a + 2; a;}"

test_ast "(do 1)" "{(1);}"
test_ast "(do (imul (add 1 2) 3))" "{(1+2)*3;}"
test_ast "(do (add 1 (imul 2 3)))" "{1+(2*3);}"
test_ast "(do (imul (add 1 2) (add 3 4)))" "{(1+2)*(3+4);}"

test_ast "(do (f))" "{f();}"
test_ast "(do (f 1))" "{f(1);}"
test_ast "(do (f 1 2))" "{f(1, 2);}"
test_ast "(do (f 1 2 3))" "{f(1, 2 ,3);}"
test_ast "(do (f 1 2 3 4))" "{f(1, 2, 3, 4);}"
test_ast "(do (f 1 2 3 4 5))" "{f(1, 2, 3,4,5);}"
test_ast "(do (f 1 2 3 4 5 6))" "{f(1, 2, 3, 4, 5, 6);}"

test_ast "(do (underscore_))" "{underscore_();}"
test_ast "(do (underscore_2))" "{underscore_2();}"
test_ast "(do (underscore_a))" "{underscore_a();}"

test_ast "(do (defvar underscore_))" "{int underscore_;}"
test_ast "(do (defvar underscore_2))" "{int underscore_2;}"
test_ast "(do (defvar underscore_a))" "{int underscore_a;}"

test_ast "(do (defvar a)(let a 42)(eval a))" "{int a;;;;;a=42;;;;a;}"

test_ast "(do (do (defvar a))(do (defvar a)))" "{{int a;}{int a;}}"

test "0" "{print_int(0);}"
test "42" "{print_int(42);}"
test "100" "{print_int(100);}"

test "1" "{0;print_int(1);}"

test "0" "{print_int(0+0);}"
test "3" "{print_int(1+2);}"
test "300" "{print_int(100 +     200);}"
test "84" "{print_int(42
+
42);}"

test "0" "{print_int(0-0);}"
test "1" "{print_int(2-1);}"
test "-1" "{print_int(1-2);}"
test "-8" "{print_int(1-2-3-4);}"

test "0" "{print_int(0*0);}"
test "2" "{print_int(1*2);}"
test "99" "{print_int(33*3);}"
test "6" "{print_int(1+2+3);}"
test "24" "{print_int(2*3*4);}"

test "7" "{print_int(1+2*3);}"
test "17" "{print_int(3*4+5);}"
test "11" "{print_int(1+2*3+4);}"

test "3" "{int a;a=1;print_int(a+2);}"
test "7" "{int a;int b;a=1;b=42;b*2;print_int(a+2*3);}"
test "9" "{print_int(1*2+1*3+2*2);}"
test "9" "{int a; int b;a=1;b=2;print_int(a*b+a*3+b*2);}"

test "2" "{int a;a=1;a=2;print_int(a);}"
test "3" "{int a;a = 1; a = a + 2; print_int(a);}"
test "30" "{
int a;
int b;
a = 1;
b = 2;
a = a + 2;
b = a + b;
a = a * b * 2;
print_int(a);}"

test "1" "{print_int((1));}"
test "9" "{print_int((1+2)*3);}"
test "7" "{print_int(1+(2*3));}"
test "21" "{print_int((1+2)*(3+4));}"

test "28" "{print_int((1-2-3)*(4-5-6));}"
test "14" "{print_int((1-(2-3))*((4-5)-6)*(0-1));}"
test "4" "{print_int((+2)*((+3)-(+5))*(-1));}"
test "4" "{print_int((-2)*((-3)-(-5))*(-1));}"

test "2" "{int a;a=2;print_int(+a);}"
test "-2" "{int a;a=2;print_int(-a);}"

test "42" "{print_int(return42());}"
test "1" "{print_int(id(1));}"
test "2" "{print_int(add(1, 1));}"
test "3" "{print_int(add3(1, 1, 1));}"
test "4" "{print_int(add4(1, 1, 1, 1));}"
test "5" "{print_int(add5(1, 1, 1, 1, 1));}"
test "6" "{print_int(add6(1, 1, 1, 1, 1, 1));}"
test "10" "{print_int(add(add(1, 2), add(3, 4)));}"

test "24" "{print_int(mul(mul(1, 2), mul(3, 4)));}"
test "1024" "{print_int(add(add(1, 2), add(3, 4)));print_int(mul(mul(1, 2), mul(3, 4)));}"
