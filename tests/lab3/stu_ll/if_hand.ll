; main 函数
define dso_local i32 @main() #0 {
	; float a = 5.555;
	%1 = alloca float;创建一个float空间，并返回
	store float 0x40163851E0000000, float* %1; 将浮点数5.555存入%1所指向的空间中

	; if(a > 1)
	%2 = load float, float* %1; 取出%1的空间中的值，也就是a
	%3 = fcmp ugt float %2, 1.000;将其和1进行比较，返回结果到%3中
	br i1 %3, label %4, label %5;如果大于，则跳转到%4，否则跳转到%5

4:	; return 233;
	ret i32 233; 返回233

5:	; return 0;
	ret i32 0; 返回0
}