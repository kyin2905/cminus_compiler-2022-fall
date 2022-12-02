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
	auto module = new Module("very easy");//新建一个编译单元的对象
	auto builder = new IRBuilder(nullptr, module);
	Type *Int32Type = Type::get_int32_type(module);
	
	auto mainFunTy = FunctionType::get(Int32Type, {});// 通过返回值类型与参数类型列表得到函数类型
	auto mainFun = Function::create(mainFunTy, "main", module);// 通过函数类型得到函数
	auto bb = BasicBlock::create(module, "entry", mainFun);
	builder->set_insert_point(bb);// 将当前插入指令点的位置设在bb
	
	//int a[10];
	auto *arrayType = ArrayType::get(Int32Type, 10);// 在内存中为数组分配空间,参数表示数组的元素类型和元素个数
	auto aAlloca = builder->create_alloca(arrayType);
	//a[0]=10;
	auto a0GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});// 获取a[0]地址
	builder->create_store(CONST_INT(10), a0GEP);                        	// a[0]=10
	a0GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});     // 更新a[0]
	//a[1] = a[0] * 2;
	auto a0Load = builder->create_load(a0GEP);                          	// 加载a[0]
	auto a0mul2 = builder->create_imul(a0Load, CONST_INT(2));           	// a[0]*2
	auto a1GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});// 获取a[1]地址
	builder->create_store(a0mul2, a1GEP);                               	// 将a[0]*2存入a[1]
	a1GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});     // 更新a[1]
	//return a[1];
	auto a1Load = builder->create_load(a1GEP);                          	// 加载a[1]
	builder->create_ret(a1Load);                                        	// 返回a[1]
	
	std::cout << module->print();
	delete module;
	return 0;
}