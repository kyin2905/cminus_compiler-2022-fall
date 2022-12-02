#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
 ConstantInt::get(num, module)

#define CONST_FP(num) \
 ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
 	auto module = new Module("very easy");
	 auto builder = new IRBuilder(nullptr, module);      // 创建IRBuilder
	 Type* Int32Type = Type::get_int32_type(module);

	 // main函数
	 auto mainFun = Function::create(FunctionType::get(Int32Type, {}),   /* 创建 main 函数 */
	 "main", module);
	 auto bb = BasicBlock::create(module, "main", mainFun);      /* 创建基本块，命名为main */
	 builder->set_insert_point(bb);                              /* 将基本块加入到builder中 */
	 
 	// 设置默认返回
 	auto retAlloca = builder->create_alloca(Int32Type);         /* 创建返回默认量 */
 	builder->create_store(CONST_INT(0), retAlloca);             /* 给默认量赋0，表示默认ret 0 */
 	
 	// 创建基本块
 	auto whileBB = BasicBlock::create(module, "whileBB", mainFun);  /* 进行while判断的基本块 */
 	auto trueBB = BasicBlock::create(module, "trueBB", mainFun);    /* 符合判断条件的基本块分支 */
 	auto falseBB = BasicBlock::create(module, "falseBB", mainFun);  /* 不符合判断条件的基本块分支 */
 	
 	// 具体执行
	auto aAlloca = builder->create_alloca(Int32Type);       /* 申请存a的空间，将地址赋值给指针aAlloca */
	auto iAlloca = builder->create_alloca(Int32Type);       /* 申请存i的空间，将地址赋值给指针iAlloca */
 	builder->create_store(CONST_INT(10), aAlloca);          /* 将值10存入a的空间 */
 	builder->create_store(CONST_INT(0), iAlloca);           /* 将值0存入i的空间 */
 	builder->create_br(whileBB);                            /* 跳转到while循环条件判断，判断是否进入循环 */
	
	//while判断的基本块
 	builder->set_insert_point(whileBB);                     /* while条件判断，设置SetInsertPoint */
 	auto i = builder->create_load(iAlloca);                 /* 取出i */
 	auto icmp = builder->create_icmp_lt(i, CONST_INT(10));  /* 判断i是否小于10，并将判断结果存到icmp中 */
 	builder->create_cond_br(icmp, trueBB, falseBB);         /* 根据icmp创建跳转语句 */

	builder->set_insert_point(trueBB);                  // if true; 分支的开始需要SetInsertPoint设置
 	i = builder->create_load(iAlloca);                  /* 取出i */
 	auto tmp = builder->create_iadd(i, CONST_INT(1));   /* 将i加1，存到暂存变量tmp中，tmp=i+1 */
 	builder->create_store(tmp, iAlloca);                /* 将tmp的值存到i中，i=tmp*/
 	auto a = builder->create_load(aAlloca);             /* 取出a */
 	i = builder->create_load(iAlloca);                  /* 取出i */
 	tmp = builder->create_iadd(a, i);                   /* 将a加i的值存到tmp中，tmp=i+a */
 	builder->create_store(tmp, aAlloca);                /* 将tmp存到a中，a=tmp */
 	builder->create_br(whileBB);                        /* 跳转到while循环条件判断，判断是否继续循环 */

 	builder->set_insert_point(falseBB);                 // if false; 分支的开始需要SetInsertPoint设置
	auto res = builder->create_load(aAlloca);           /* 取出a的值，存到res中，res=a */
 	builder->create_ret(res);                           /* 将res返回，即return res */

 	std::cout << module->print();
	delete module;

	return 0;
}