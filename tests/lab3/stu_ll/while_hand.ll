;main函数和左括号
define dso_local i32 @main() #0 {

;int a
%1 = alloca i32;开辟int类型空间，存储a

;int i
%2 = alloca i32;开辟int类型空间，存储i

;a = 10
store i32 10, i32* %1;%1赋值为10，也就是a = 10

;i = 0
store i32 0, i32* %2;%2赋值为0，也就是i = 0
br label %3;跳转到3

;下面是while语句while(i < 10){
3:
%4 = load i32, i32* %2;把%2也就是i的值加载到%4
;比较i和10的大小，如果小于则5为真，否则为否
%5 = icmp slt i32 %4, 10
;5若真则跳转到6，5若否则跳转到11
br i1 %5, label %6, label %11

;i = i + 1
6:
%7 = add nsw i32 %4, 1;%7保存加一后的值
store i32 %7, i32* %2;存储回%2

;a = a + i
%8 = load i32, i32* %1;a的数据加载到%8
%9 = load i32, i32* %2;i的数据加载到%9
%10 = add nsw i32 %8, %9;%10保存相加的数据
store i32 %10, i32* %1;存储回%1
br label %3;跳转到3，也就是继续判断while循环是否成立

;}

;return a

11:
%12 = load i32, i32* %1;a的数据加载到%12
ret i32 %12;返回%12

;}，右括号
}