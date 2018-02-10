#! /bin/bash -x

konoha=./konoha

compile() {
    echo "$1" | "$konoha" -o tmp/out.s
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
    ret=$?
    if [ $ret != 0 ]; then
        echo "got nonzero return code($ret)"
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
    : ok
}

test_ast "(defun f<int()> () (do ))" "int f() {}"
test_ast "(defun f<int(int)> (n) (do ))" "int f(int n) {}"
test_ast "(defun f<int(int, int)> (n, m) (do ))" "int f(int n, int m) {}"
test_ast "(defun f<int(int, int, int)> (a, b, c) (do ))" "int f(int a, int b, int c) {}"
test_ast "(defun f<int(int, int, int, int)> (a, b, c, d) (do ))" "int f(int a, int b, int c, int d) {}"
test_ast "(defun f<int(int, int, int, int, int)> (a, b, c, d, e) (do ))" "int f(int a, int b, int c, int d, int e) {}"
test_ast "(defun f<int(int, int, int, int, int, int)> (a, b, c, d, e, _) (do ))" "int f(int a, int b, int c, int d, int e, int _) {}"

test_ast "(defun f<int()> () (do 0))" "int f() {0;}"
test_ast "(defun f<int()> () (do 42))" "int f() {42;}"
test_ast "(defun f<int()> () (do 100))" "int f() {100;}"

test_ast "(defun f<int()> () (do 0))" "    int     f(         )   {    0     ;}"

test_ast "(defun f<int()> () (do (add 0 0)))" "int f() {0+0;}"
test_ast "(defun f<int()> () (do (add 1 2)))" "int f() {1+2;}"
test_ast "(defun f<int()> () (do (add 100 200)))" "int f() {100 +     200;}"
test_ast "(defun f<int()> () (do (add 42 42)))" "int f() {42
+
42;}"

test_ast "(defun f<int()> () (do (sub 0 0)))" "int f() {0-0;}"
test_ast "(defun f<int()> () (do (sub 2 1)))" "int f() {2-1;}"
test_ast "(defun f<int()> () (do (sub 1 2)))" "int f() {1-2;}"
test_ast "(defun f<int()> () (do (sub (sub (sub 1 2) 3) 4)))" "int f() {1-2-3-4;}"

test_ast "(defun f<int()> () (do (imul 0 0)))" "int f() {0*0;}"
test_ast "(defun f<int()> () (do (imul 1 2)))" "int f() {1*2;}"
test_ast "(defun f<int()> () (do (imul 33 3)))" "int f() {33*3;}"

test_ast "(defun f<int()> () (do (idivl 33 3)))" "int f() {33/3;}"

test_ast "(defun f<int()> () (do (add (add 1 2) 3)))" "int f() {1+2+3;}"
test_ast "(defun f<int()> () (do (imul (imul 2 3) 4)))" "int f() {2*3*4;}"

test_ast "(defun f<int()> () (do (add 1 (imul 2 3))))" "int f() {1+2*3;}"
test_ast "(defun f<int()> () (do (add (imul 3 4) 5)))" "int f() {3*4+5;}"
test_ast "(defun f<int()> () (do (add (add 1 (imul 2 3)) 4)))" "int f() {1+2*3+4;}"

test_ast "(defun f<int()> () (do (defvar a)))" "int f() { int a; }"
test_ast "(defun f<int()> () (do (defvar a)(eval a)))" "int f() {int a; a;}"
test_ast "(defun f<int()> () (do (defvar a)(let a 0)))" "int f() {int a;a=0;}"
test_ast "(defun f<int()> () (do (defvar a)(let a (add 1 2))))" "int f() {int a; a=1+2;}"
test_ast "(defun f<int()> () (do (defvar a)(defvar b)(let a 0)(let b 0)))" "int f() {int a; int b; a=0; b=0;}"
test_ast "(defun f<int()> () (do (defvar a)(let a 0)(defvar b)(let b 0)))" "int f() {int a; a=0; int b; b=0;}"

test_ast "(defun f<int()> () (do (defvar a)(defvar b)(let a 1)(let b 2)(add (add (imul (eval a) (eval b)) (imul (eval a) 3)) (imul (eval b) 2))))" "int f() {int a; int b;a=1;b=2;a*b+a*3+b*2;}"
test_ast "(defun f<int()> () (do (defvar a)(let a 1)(let a (add (eval a) 2))(eval a)))" "int f() {int a;a = 1; a = a + 2; a;}"

test_ast "(defun f<int()> () (do 1))" "int f() {(1);}"
test_ast "(defun f<int()> () (do (imul (add 1 2) 3)))" "int f() {(1+2)*3;}"
test_ast "(defun f<int()> () (do (add 1 (imul 2 3))))" "int f() {1+(2*3);}"
test_ast "(defun f<int()> () (do (imul (add 1 2) (add 3 4))))" "int f() {(1+2)*(3+4);}"

test_ast "(defun f<int()> () (do (f)))" "int f() {f();}"
test_ast "(defun f<int()> () (do (f 1)))" "int f() {f(1);}"
test_ast "(defun f<int()> () (do (f 1 2)))" "int f() {f(1, 2);}"
test_ast "(defun f<int()> () (do (f 1 2 3)))" "int f() {f(1, 2 ,3);}"
test_ast "(defun f<int()> () (do (f 1 2 3 4)))" "int f() {f(1, 2, 3, 4);}"
test_ast "(defun f<int()> () (do (f 1 2 3 4 5)))" "int f() {f(1, 2, 3,4,5);}"
test_ast "(defun f<int()> () (do (f 1 2 3 4 5 6)))" "int f() {f(1, 2, 3, 4, 5, 6);}"

test_ast "(defun f<int()> () (do (underscore_)))" "int f() {underscore_();}"
test_ast "(defun f<int()> () (do (underscore_2)))" "int f() {underscore_2();}"
test_ast "(defun f<int()> () (do (underscore_a)))" "int f() {underscore_a();}"

test_ast "(defun f<int()> () (do (defvar underscore_)))" "int f() {int underscore_;}"
test_ast "(defun f<int()> () (do (defvar underscore_2)))" "int f() {int underscore_2;}"
test_ast "(defun f<int()> () (do (defvar underscore_a)))" "int f() {int underscore_a;}"

test_ast "(defun f<int()> () (do (defvar a)(let a 42)(eval a)))" "int f() {int a;;;;;a=42;;;;a;}"

test_ast "(defun f<int()> () (do (do (defvar a))(do (defvar a))))" "int f() {{int a;}{int a;}}"

test_ast "(defun f<int()> () (do (defvar a)))" "int f() {
// int abc;
int a;
}"
test_ast "(defun f<int()> () (do (defvar a)))" "int f() {
/* int abc; */
int a;
}"
test_ast "(defun f<int()> () (do (defvar a)))" "int f() {
/* int* abc; 1 / 2 * 3 + 4 */
int a;
}"

test_ast "(defun f<char()> () (do (defvar a)(let a 0)))" "char f() {char a;a=0;}"

test_ast "(defun f<int()> () (do (return 42)))" "int f() {return 42;}"
test_ast "(defun f<int()> () (do (return 42)))" "int f() {return(42);}"

test_ast "(defun f<int()> () (do (return 42)))(defun g<int()> () (do (return (f))))(defun main<int()> () (do (print_int (g))))" "int f() { return 42;} int g() {return f();} int main() { print_int(g()); }"

test_ast "(defun main<int()> () (do (if (0) ((do (return (print_int 42)))) ((return (print_int 5)))))" "int main() { if(0) { return print_int(42);} else return print_int(5); }"
test_ast "(defun main<int()> () (do (if (1) ((do (return (print_int 42)))) ((return (print_int 5)))))" "int main() { if(1) { return print_int(42);} else return print_int(5); }"
test_ast "(defun main<int()> () (do (if (1) ((if (0) ((print_int 0)) ((print_int 1)))))" "int main(){
if(1)
 if(0) print_int(0);
 else print_int(1);
}"

test_ast "(defun main<int()> () (do (defvar a)(do (let a 1))))" "int main() {int a; { a = 1; } }"

test "0" "int main() {print_int(0);}"
test "42" "int main() {print_int(42);}"
test "100" "int main() {print_int(100);}"

test "1" "int main() {0;print_int(1);}"

test "0" "int main() {print_int(0+0);}"
test "3" "int main() {print_int(1+2);}"
test "300" "int main() {print_int(100 +     200);}"
test "84" "int main() {print_int(42
+
42);}"

test "0" "int main() {print_int(0-0);}"
test "1" "int main() {print_int(2-1);}"
test "-1" "int main() {print_int(1-2);}"
test "-8" "int main() {print_int(1-2-3-4);}"

test "0" "int main() {print_int(0*0);}"
test "2" "int main() {print_int(1*2);}"
test "99" "int main() {print_int(33*3);}"
test "6" "int main() {print_int(1+2+3);}"
test "24" "int main() {print_int(2*3*4);}"

test "0" "int main() {print_int(0/1);}"
test "3" "int main() {print_int(10/3);}"
test "11" "int main() {print_int(33/3);}"

test "7" "int main() {print_int(1+2*3);}"
test "17" "int main() {print_int(3*4+5);}"
test "11" "int main() {print_int(1+2*3+4);}"

test "3" "int main() {int a;a=1;print_int(a+2);}"
test "7" "int main() {int a;int b;a=1;b=42;b*2;print_int(a+2*3);}"
test "9" "int main() {print_int(1*2+1*3+2*2);}"
test "9" "int main() {int a; int b;a=1;b=2;print_int(a*b+a*3+b*2);}"

test "2" "int main() {int a;a=1;a=2;print_int(a);}"
test "3" "int main() {int a;a = 1; a = a + 2; print_int(a);}"
test "30" "int main() {
int a;
int b;
a = 1;
b = 2;
a = a + 2;
b = a + b;
a = a * b * 2;
print_int(a);}"
test "1" "int main() {int a;a = 2; a = a - 1; print_int(a);}"

test "1" "int main() {print_int((1));}"
test "9" "int main() {print_int((1+2)*3);}"
test "7" "int main() {print_int(1+(2*3));}"
test "21" "int main() {print_int((1+2)*(3+4));}"

test "28" "int main() {print_int((1-2-3)*(4-5-6));}"
test "14" "int main() {print_int((1-(2-3))*((4-5)-6)*(0-1));}"
test "4" "int main() {print_int((+2)*((+3)-(+5))*(-1));}"
test "4" "int main() {print_int((-2)*((-3)-(-5))*(-1));}"

test "2" "int main() {int a;a=2;print_int(+a);}"
test "-2" "int main() {int a;a=2;print_int(-a);}"

test "42" "int main() {print_int(return42());}"
test "1" "int main() {print_int(id(1));}"
test "2" "int main() {print_int(add(1, 1));}"
test "3" "int main() {print_int(add3(1, 1, 1));}"
test "4" "int main() {print_int(add4(1, 1, 1, 1));}"
test "5" "int main() {print_int(add5(1, 1, 1, 1, 1));}"
test "6" "int main() {print_int(add6(1, 1, 1, 1, 1, 1));}"
test "10" "int main() {print_int(add(add(1, 2), add(3, 4)));}"

test "24" "int main() {print_int(mul(mul(1, 2), mul(3, 4)));}"
test "1024" "int main() {print_int(add(add(1, 2), add(3, 4)));print_int(mul(mul(1, 2), mul(3, 4)));}"

test "42" "int f() { return 42;} int g() {return f();} int main() { print_int(g()); }"

test "5" "int main() { if(0) { return print_int(42);} else return print_int(5); }"
test "42" "int main() { if(1) { return print_int(42);} else return print_int(5); }"
test "42" "int main() { if(42) { return print_int(42);} else return print_int(5); }"
test "42" "int main() { if(-1) { return print_int(42);} else return print_int(5); }"
test "1" "int main(){
if(1)
 if(0) print_int(0);
 else print_int(1);
}"

test "424140393837363534333231302928272625242322212019181716151413121110987654321" "int main(){ int a; a = 42; while (a) {print_int(a); a = a - 1;}}"
