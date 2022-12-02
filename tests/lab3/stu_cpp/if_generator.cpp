#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG	// 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;	// 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module)	// 得到常数值的表示,方便后面多次用到

int main()
{
	auto module = new Module("very easy");
	auto builder = new IRBuilder(nullptr, module);
	Type *Int32Type = Type::get_int32_type(module);
	Type *FloatType = Type::get_float_type(module);
	
	//main函数
	auto mainFunTy = FunctionType::get(Int32Type, {});	// 通过返回值类型与参数类型列表得到函数类型
	auto mainFun = Function::create(mainFunTy, "main", module);	// 通过函数类型得到函数
	auto bb = BasicBlock::create(module, "entry", mainFun);
	builder->set_insert_point(bb);	// 将当前插入指令点的位置设在bb
	
	auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置

	//float a =5.555;
	auto aAlloca = builder->create_alloca(FloatType);     // 在内存中分配浮点数a的位置
	builder->create_store(CONST_FP(5.555), aAlloca);
	auto aLoad = builder->create_load(aAlloca);	//load上来
	
	//if(a>1)
	auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1));		//比较大小
	auto trueBB = BasicBlock::create(module, "trueBB", mainFun);	// true分支
	auto falseBB = BasicBlock::create(module, "falseBB", mainFun);	// false分支
	auto retBB = BasicBlock::create(module, "", mainFun);	// return分支,提前create,以便true分支可以br

	auto br = builder->create_cond_br(fcmp, trueBB, falseBB);  // 条件BR
	DEBUG_OUTPUT // 我调试的时候故意留下来的,以醒目地提醒你这个调试用的宏定义方法
	
	//return 233;
	builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
	builder->create_store(CONST_INT(233), retAlloca);
	builder->create_br(retBB);  // br retBB

	//return 0;
	builder->set_insert_point(falseBB);  // if false
	builder->create_store(CONST_INT(0), retAlloca);
	builder->create_br(retBB);  // br retBB

	builder->set_insert_point(retBB);  // ret分支
	auto retLoad = builder->create_load(retAlloca);
	builder->create_ret(retLoad);
	
	std::cout << module->print();
	delete module;
	return 0;
}