;main函数中，dso_local i32 @main()表示在main函数中
define dso_local i32 @main() #0 {

;对应语句int a[10]
%1 = alloca [10 x i32]	;创建数组，可容纳10个int类型的数据

;a[0] = 10;
%2 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i64 0, i64 0	;%2为a[0]，这里是获取a[0]的指针
store i32 10, i32* %2	;%2赋值为10，a[0]赋值为10

;a[1] = a[0] * 2;
%3 = load i32, i32* %2	;%3加载%2的数据
%4 = mul nsw i32 %3, 2	;%4赋值为%3的二倍
;%4就是a[0] * 2
%5 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i64 0, i64 1	;%5存储a[1]
store i32 %4, i32* %5	;把%4的值赋值给%5，完成a[1]赋值

;return a[1];
%6 = load i32, i32* %5	;%6存储%5
ret i32 %6		;返回%6

;main函数的右括号
}