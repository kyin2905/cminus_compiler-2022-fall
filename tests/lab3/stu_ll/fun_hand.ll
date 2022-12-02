;这里的意思是一个函数int callee(int a){
define dso_local i32 @callee(i32 %0) #0{
	;return 2 * a
	%2 = mul i32 %0, 2; 将参数%1乘以2存入%2中，mul代表乘法
	ret i32 %2; 返回%2
}

;main函数{
define dso_local i32 @main() #0 {

;return callee(110);
%1 = call i32 @callee(i32 110)	;%用1存储callee(110)的返回值
ret i32 %1			;返回%1

;右括号
}