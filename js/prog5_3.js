// 测试错误提示

fun foo() {
    a = 4;
}

fun bar() {
    foo();
}

fun work() {
    bar();
}

work();