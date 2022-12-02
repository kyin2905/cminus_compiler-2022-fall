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

 	// callee函数，创建函数
 	std::vector<Type*> Ints(1, Int32Type);                  /* 函数参数类型的vector，内含1个int类型 */
 	auto calleeFunTy = FunctionType::get(Int32Type, Ints);  /* 通过返回值类型与参数类型列表得到函数类型 */
 	auto calleeFun = Function::create(calleeFunTy,          /* 由函数类型得到函数 */
     	"callee", module);
 	auto bb = BasicBlock::create(module, "fun", calleeFun); /* 创建基本块，命名为fun */
 	builder->set_insert_point(bb);                          /* 将基本块插入builder中 */
 	// 传参
 	auto aAlloca = builder->create_alloca(Int32Type);       /* 在内存中分配参数a的位置 */
 	std::vector<Value*> args;                          /* 获取callee函数的形参,通过Function中的iterator */
 	for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
     		args.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
 	}
 	builder->create_store(args[0], aAlloca);                /* 存储参数a */
 	// 具体执行
 	auto aLoad = builder->create_load(aAlloca);             /* 将参数a存到变量aLoad中 */
 	auto res = builder->create_imul(aLoad, CONST_INT(2));   /* 将值乘以2存入变量res中 */
 	builder->create_ret(res);                               /* 创建返回，将res返回 */

 	// main函数
 	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),   /* 创建 main 函数 */
     	"main", module);
 	bb = BasicBlock::create(module, "main", mainFun);           /* 创建基本块，命名为main */
 	builder->set_insert_point(bb);                              /* 将基本块加入到builder中 */
 	// 设置默认返回
 	auto retAlloca = builder->create_alloca(Int32Type);         /* 创建返回默认量 */
 	builder->create_store(CONST_INT(0), retAlloca);             /* 给默认量赋0，表示默认ret 0 */
 	// 具体执行
 	auto call = builder->create_call(calleeFun, {CONST_INT(110)});/* 调用函数calleeFun，将结果存到变量call中 */
 	builder->create_ret(call);      /* 返回结果值 */

 	std::cout << module->print();
 	delete module;

 	return 0;
}