#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_INT(num) \
 ConstantInt::get(num, module.get())     /* 增加一个有关整型的宏 */
 //以num值来创建常数类
#define CONST_FP(num) \
 ConstantFP::get((float)num, module.get())
 //以num值来创建浮点数类
#define CONST_ZERO(type) \
 ConstantZero::get(type, module.get())   /* 此处要修改为type */
 //用于全局变量初始化的常量0值

#define Int32Type \
 Type::get_int32_type(module.get())      /* 获取int32类型 */
#define FloatType \
 Type::get_float_type(module.get())      /* 获取float类型 */

#define checkInt(num) \
 num->get_type()->is_integer_type()      /* 整型判断 */
#define checkFloat(num) \
 num->get_type()->is_float_type()        /* 浮点型判断 */
#define checkPointer(num) \
 num->get_type()->is_pointer_type()      /* 指针类型判断 */

// You can define global variables here
// to store state
Value* Res;                     /* 存储返回的结果 */
Value* arg;                     /* 存储参数指针，用于Param的处理 */
bool need_as_address = false;   /* 标志是返回值还是返回地址 */

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

//下面的类型在include/ast.hpp有定义
//lab4和lab3的区别就是lab3是利用llvm接口来生成中间代码，而lab4直接写一个编译器将cminus转成中间代码

/* Program, 程序, program->declaration-list */
void CminusfBuilder::visit(ASTProgram &node) {  
//ASTProgram有一个std::vector<std::shared_ptr<ASTDeclaration>>类型的向量，我们要对这个向量进行遍历
 for (auto decl : node.declarations)         /* 遍历declaration-list子结点 */
     decl->accept(*this);                    /* 处理每一个declaration */
}

/* Num，对数值进行处理 */
void CminusfBuilder::visit(ASTNum &node) {     
//ASTNum中有CminusType type;类型，还有一个联合体union
 if (node.type == TYPE_INT)                  /* 若为整型 */
	//调用ConstantInt中的API
     Res = ConstantInt::get(node.i_val, module.get());   /* 获取结点中存储的整型数值 */
 else if (node.type == TYPE_FLOAT)           /* 若为浮点型 */
 	//调用ConstantFP中的API
     Res = ConstantFP::get(node.f_val, module.get());    /* 获取结点中存储的浮点型数值 */
}

/* Var-Declaration, 变量声明, var-declaration -> type-specifier ID | type-specifier ID [INTEGER] */
void CminusfBuilder::visit(ASTVarDeclaration &node) {              
//ASTVarDeclaration有两个成员变量：CminusType type和std::shared_ptr<ASTNum> num;
    Type* tmpType;                  /* 类型暂存变量，用于存储变量的类型，用于后续申请空间 */
    if (node.type == TYPE_INT)      /* 若为整型 */
        tmpType = Int32Type;        /* 则type为整数类型 */
    else if (node.type == TYPE_FLOAT)   /* 则为浮点型 */
        tmpType = FloatType;            /* 则type为浮点类型 */
    if (node.num != nullptr) {          /* 若为数组类型 */
        /* 获取需开辟的对应大小的空间的类型指针 */
        auto* arrayType = ArrayType::get(tmpType, node.num->i_val); /* 获取对应的数组Type */
        auto initializer = CONST_ZERO(tmpType);                     /* 全局变量初始化为0 */
        Value* arrayAlloca;             /* 存储申请到的数组空间的地址 */
        if (scope.in_global())          /* 若为全局数组，则开辟全局数组 */
            arrayAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);
        else                            /* 若不是全局数组，则开辟局部数组 */
            arrayAlloca = builder->create_alloca(arrayType);
        scope.push(node.id, arrayAlloca);/* 将获得的数组变量加入域 */
    }
    else {                              /* 若不是数组类型 */
        auto initializer = CONST_ZERO(tmpType); /* 全局变量初始化为0 */
        Value* varAlloca;               /* 存储申请到的变量空间的地址 */
        if (scope.in_global())          /* 若为全局变量，则申请全局空间 */
            varAlloca = GlobalVariable::create(node.id, module.get(), tmpType, false, initializer);
        else                            /* 若不是全局变量，则申请局部空间 */
            varAlloca = builder->create_alloca(tmpType);
        scope.push(node.id, varAlloca); /* 将获得变量加入域 */
    }
}

/* Fun-Declaration, 函数声明, fun-declaration -> type-specifier ID ( params ) compound-stmt */
void CminusfBuilder::visit(ASTFunDeclaration &node) {  
//考虑函数返回类型+函数名+参数列表+复合语句（局部声明与语句列表） 
    Type* retType;               /* 存储返回类型 */
    /* 根据不同的返回类型，设置retType */
    if (node.type == TYPE_INT) { retType = Int32Type; }
    if (node.type == TYPE_FLOAT) { retType = FloatType; }
    if (node.type == TYPE_VOID) { retType = Type::get_void_type(module.get()); }
    /* 根据函数声明，构造形参列表（此处的形参即参数的类型） */
    std::vector<Type*> paramsType;  /* 参数类型列表 */
    for (auto param : node.params) {
        if (param->isarray) {       /* 若参数为数组形式，则存入首地址指针 */
            if (param->type == TYPE_INT)        /* 若为整型 */
                paramsType.push_back(Type::get_int32_ptr_type(module.get()));
            else if(param->type == TYPE_FLOAT)  /* 若为浮点型 */
                paramsType.push_back(Type::get_float_ptr_type(module.get()));
        }
        else {                  /* 若为单个变量形式，则存入对应类型 */
            if (param->type == TYPE_INT)        /* 若为整型 */
                paramsType.push_back(Int32Type);
            else if (param->type == TYPE_FLOAT) /* 若为浮点型 */
                paramsType.push_back(FloatType);
        }
    }
    auto funType = FunctionType::get(retType, paramsType);/* retType返回结构，paramsType函数形参结构 */
    auto function = Function::create(funType, node.id, module.get());   /* 创建函数 */
    scope.push(node.id, function);  /* 将函数加入到域 */
    scope.enter();                  /* 进入此函数作用域 */
    auto bb = BasicBlock::create(module.get(), node.id + "_entry", function);/* 创建基本块 */
    builder->set_insert_point(bb);  /* 将基本块加入到builder中 */
    /* 函数传参，要将实参和形参进行匹配 */
    std::vector<Value*> args;       /* 创建vector存储实参 */
    for (auto arg = function->arg_begin();arg != function->arg_end();arg++) {/* 遍历实参列表 */
        args.push_back(*arg);       /* 将实参加入vector */
    }
    for (int i = 0;i < node.params.size();i++) {    /* 遍历形参列表（=遍历实参列表） */
        auto param = node.params[i];        /* 取出对应形参 */
        arg = args[i];                      /* 取出对应实参 */
        param->accept(*this);           /* 调用param的accept进行处理 */
    }
    node.compound_stmt->accept(*this);      /* 处理函数体内语句compound-stmt */
    //判断返回值的类型，根据对应的返回值类型，执行ret操作
    if (builder->get_insert_block()->get_terminator() == nullptr) {
        if (function->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (function->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    scope.exit();                       /* 退出此函数作用域 */
}

/* Param, 参数 */
void CminusfBuilder::visit(ASTParam &node) {  
//成员变量：CminusType type;std::string id;bool isarray;      
 Value* paramAlloca; /* 该参数的存储空间 */
 if (node.isarray) { /* 若为数组 */
     if (node.type == TYPE_INT)        /* 若为整型数组，则开辟整型数组存储空间 */
         paramAlloca = builder->create_alloca(Type::get_int32_ptr_type(module.get()));
     else if (node.type == TYPE_FLOAT) /* 若为浮点数数组，则开辟浮点数数组存储空间 */
         paramAlloca = builder->create_alloca(Type::get_float_ptr_type(module.get()));
 }
 else {              /* 若不是数组 */
     if (node.type == TYPE_INT)        /* 若为整型，则开辟整型存储空间 */
         paramAlloca = builder->create_alloca(Int32Type);
     else if (node.type == TYPE_FLOAT) /* 若为浮点数，则开辟浮点数存储空间 */
         paramAlloca = builder->create_alloca(FloatType);
 }
 builder->create_store(arg, paramAlloca);    /* 将实参load到开辟的存储空间中 */
 scope.push(node.id, paramAlloca);           /* 将参数push到域中 */
}

/* CompoundStmt, 函数体语句, compound-stmt -> {local-declarations statement-list} */
void CminusfBuilder::visit(ASTCompoundStmt &node) {
 scope.enter();      /* 进入函数体内的作用域 */
 for (auto local_declaration : node.local_declarations)   /* 遍历 */
     local_declaration->accept(*this);   /* 处理每一个局部声明 */
 for (auto statement : node.statement_list)              /* 遍历 */
     statement->accept(*this);           /* 处理每一个语句 */
 scope.exit();       /* 退出作用域 */
}

/* ExpressionStmt, 表达式语句, expression-stmt -> expression;
 *                                              | ; */
void CminusfBuilder::visit(ASTExpressionStmt &node) {       /*  */
    if (node.expression != nullptr)     /* 若对应表达式存在 */
        node.expression->accept(*this); /* 则处理该表达式 */
}

/* SelectionStmt, if语句, selection-stmt -> if (expression) statement
 *                                        | if (expression) statement else statement */
void CminusfBuilder::visit(ASTSelectionStmt &node) {        
//成员变量：std::shared_ptr<ASTExpression> expression;
//std::shared_ptr<ASTStatement> if_statement;
//std::shared_ptr<ASTStatement> else_statement;
    auto function = builder->get_insert_block()->get_parent();  /* 获得当前所对应的函数 */
    node.expression->accept(*this);         /* 处理条件判断对应的表达式，得到返回值存到expression中 */
    auto resType = Res->get_type();         /* 获取表达式得到的结果类型 */
    Value* TrueFalse;                       /* 存储if判断的结果 */
    if (resType->is_integer_type()) {       /* 若结果为整型，则针对整型进行处理(bool类型视为整型) */
        auto intType = Int32Type;  
        TrueFalse = builder->create_icmp_gt(Res, CONST_ZERO(intType));  /* 大于0视为true */
    }
    else if (resType->is_float_type()) {    /* 若结果为浮点型，则针对浮点数进行处理 */
        auto floatType = FloatType;
        TrueFalse = builder->create_fcmp_gt(Res, CONST_ZERO(floatType));/* 大于0视为true */
    }
    if (node.else_statement != nullptr) {   /* 若存在else语句 */
        auto trueBB = BasicBlock::create(module.get(), "true", function);   /* 创建符合条件块 */
        auto falseBB = BasicBlock::create(module.get(), "false", function); /* 创建else块 */
        builder->create_cond_br(TrueFalse, trueBB, falseBB);    /* 设置跳转语句 */

        builder->set_insert_point(trueBB);  /* 符合if条件的块 */
        node.if_statement->accept(*this);   /* 处理符合条件后要执行的语句 */
        auto curTrueBB = builder->get_insert_block();   /* 将块加入 */

        builder->set_insert_point(falseBB); /* else的块 */
        node.else_statement->accept(*this); /* 处理else语句 */
        auto curFalseBB = builder->get_insert_block();  /* 将块加入 */

        /* 处理返回，避免跳转到对应块后无return */
        auto trueTerm = builder->get_insert_block()->get_terminator();  /* 判断true语句中是否存在ret语句 */
        auto falseTerm = builder->get_insert_block()->get_terminator(); /* else语句中是否存在ret语句 */
        BasicBlock* retBB;
        if (trueTerm == nullptr || falseTerm == nullptr) {  /* 若有一方不存在return语句，则需要创建返回块 */
            retBB = BasicBlock::create(module.get(), "ret", function);  /* 创建return块 */
            builder->set_insert_point(retBB);               /* return块（即后续语句） */
        }
        if (trueTerm == nullptr) {          /* 若符号条件后要执行的语句中不存在return */
            builder->set_insert_point(curTrueBB);   /* 则设置跳转 */
            builder->create_br(retBB);              /* 跳转到刚刚设置的return块 */
        }
        if (falseTerm == nullptr) {         /* 若else语句中不存在return */
            builder->set_insert_point(curFalseBB);  /* 则设置跳转 */
            builder->create_br(retBB);              /* 跳转到刚刚设置的return块 */
        }
    }
    else {  /* 若不存在else语句，则只需要设置true语句块和后续语句块即可 */
        auto trueBB = BasicBlock::create(module.get(), "true", function);   /* true语句块 */
        auto retBB = BasicBlock::create(module.get(), "ret", function);     /* 后续语句块 */
        builder->create_cond_br(TrueFalse, trueBB, retBB);  /* 根据条件设置跳转指令 */

        builder->set_insert_point(trueBB);  /* true语句块 */
        node.if_statement->accept(*this);   /* 执行条件符合后要执行的语句 */
        if (builder->get_insert_block()->get_terminator() == nullptr)   /* 补充return（同上） */
            builder->create_br(retBB);      /* 跳转到刚刚设置的return块 */
        builder->set_insert_point(retBB);   /* return块（即后续语句） */
    }
}

/* IterationStmt, while语句, iteration-stmt -> while (expression) statement */
void CminusfBuilder::visit(ASTIterationStmt &node) {    
    auto function = builder->get_insert_block()->get_parent();  /* 获得当前所对应的函数 */
    auto conditionBB = BasicBlock::create(module.get(), "condition", function); /* 创建条件判断块 */
    auto loopBB = BasicBlock::create(module.get(), "loop", function);           /* 创建循环语句块 */
    auto retBB = BasicBlock::create(module.get(), "ret", function);             /* 创建后续语句块 */
    builder->create_br(conditionBB);        /* 跳转到条件判断块 */

    builder->set_insert_point(conditionBB); /* 条件判断块 */
    node.expression->accept(*this);         /* 处理条件判断对应的表达式，得到返回值存到expression中 */
    auto resType = Res->get_type();         /* 获取表达式得到的结果类型 */
    Value* TrueFalse;                       /* 存储if判断的结果 */
    if (resType->is_integer_type()) {       /* 若结果为整型，则针对整型进行处理(bool类型视为整型) */
        auto intType = Int32Type;
        TrueFalse = builder->create_icmp_gt(Res, CONST_ZERO(intType));  /* 大于0视为true */
    }
    else if (resType->is_float_type()) {    /* 若结果为浮点型，则针对浮点数进行处理 */
        auto floatType = FloatType;
        TrueFalse = builder->create_fcmp_gt(Res, CONST_ZERO(floatType));/* 大于0视为true */
    }
    builder->create_cond_br(TrueFalse, loopBB, retBB);  /* 设置条件跳转语句 */

    builder->set_insert_point(loopBB);      /* 循环语句执行块 */
    node.statement->accept(*this);          /* 执行对应的语句 */
    if (builder->get_insert_block()->get_terminator() == nullptr)   /* 若无返回，则补充跳转 */
        builder->create_br(conditionBB);    /* 跳转到条件判断语句 */

    builder->set_insert_point(retBB);       /* return块（即后续语句） */
}

/* ReturnStmt, 返回语句, return-stmt -> return;
 *                                    | return expression; */
void CminusfBuilder::visit(ASTReturnStmt &node) {   
    auto function = builder->get_insert_block()->get_parent();  /* 获得当前所对应的函数 */
    auto retType = function->get_return_type();     /* 获取返回类型 */
    if (retType->is_void_type()) {      /* 如果是void */
        builder->create_void_ret();     /* 则创建void返回，随后return，无需后续操作 */
        return;
    }
    /* 处理非void的情况 */
    node.expression->accept(*this);     /* 处理条件判断对应的表达式，得到返回值存到expression中 */
    auto resType = Res->get_type();     /* 获取表达式得到的结果类型 */
    /* 处理expression返回的结果和需要return的结果类型不匹配的问题 */
    if (retType->is_integer_type() && resType->is_float_type()) 
        Res = builder->create_fptosi(Res, Int32Type);
    if (retType->is_float_type() && resType->is_integer_type())
        Res = builder->create_sitofp(Res, Int32Type);
    builder->create_ret(Res);           /* 创建return，将expression的结果进行返回 */
}

/* Var, 变量引用, var -> ID
 *                     | ID [expression] */
void CminusfBuilder::visit(ASTVar &node) { 
//成员变量std::string id;
//std::shared_ptr<ASTExpression> expression;// nullptr if var is of int type    
    auto var = scope.find(node.id);             /* 从域中取出对应变量 */
    bool should_return_lvalue = need_as_address;    /* 判断是否需要返回地址（即是否由赋值语句调用） */
    need_as_address = false;            /* 重置全局变量need_as_address */
    Value* index = CONST_INT(0);        /* 初始化index */
    if (node.expression != nullptr) {   /* 若有expression，代表不是int类型的引用*/
        node.expression->accept(*this); /* 处理expression，得到结果Res */
        auto res = Res;                 /* 存储结果 */
        if (checkFloat(res))            /* 判断结果是否为浮点数 */
            res = builder->create_fptosi(res, Int32Type);   /* 若是，则矫正为整数 */
        index = res;                    /* 赋值给index，表示数组下标 */
        /* 判断下标是否为负数。若是，则调用neg_idx_except函数进行处理 */
        auto function = builder->get_insert_block()->get_parent();  /* 获取当前函数 */
        auto indexTest = builder->create_icmp_lt(index, CONST_ZERO(Int32Type)); /* test是否为负数 */
        auto failBB = BasicBlock::create(module.get(), node.id + "_failTest", function);/* fail块 */
        auto passBB = BasicBlock::create(module.get(), node.id + "_passTest", function);/* pass块 */
        builder->create_cond_br(indexTest, failBB, passBB); /* 设置跳转语句 */

        builder->set_insert_point(failBB);  /* fail块，即下标为负数 */
        auto fail = scope.find("neg_idx_except");               /* 取出neg_idx_except函数 */
        builder->create_call(static_cast<Function*>(fail), {}); /* 调用neg_idx_except函数进行处理 */
        builder->create_br(passBB);         /* 跳转到pass块 */

        builder->set_insert_point(passBB);  /* pass块 */
        if (var->get_type()->get_pointer_element_type()->is_array_type())   /* 若为指向数组的指针 */
            var = builder->create_gep(var, { CONST_INT(0), index });    /* 则进行两层寻址（原因在上一实验中已说明） */
        else {
            if (var->get_type()->get_pointer_element_type()->is_pointer_type()) /* 若为指针 */
                var = builder->create_load(var);        /* 则取出指针指向的元素 */
            var = builder->create_gep(var, { index });  /* 进行一层寻址（因为此时并非指向数组） */
        }
        if (should_return_lvalue) {         /* 若要返回值 */
            Res = var;                      /* 则返回var对应的地址 */
            need_as_address = false;        /* 并重置全局变量need_as_address */
        }
        else 
            Res = builder->create_load(var);/* 否则则进行load */
        return;
    }
    /* 处理无expression的情况 */
    if (should_return_lvalue) { /* 若要返回值 */
        Res = var;              /* 则返回var对应的地址 */
        need_as_address = false;/* 并重置全局变量need_as_address */
    }
    else {                      /* 否则 */
        if (var->get_type()->get_pointer_element_type()->is_array_type())   /* 若指向数组 */
            Res = builder->create_gep(var, { CONST_INT(0), CONST_INT(0) }); /* 则寻址 */
        else
            Res = builder->create_load(var);/* 否则则进行load */
    }
}

/* AssignExpression, 赋值语句, var = expression */
void CminusfBuilder::visit(ASTAssignExpression &node) { 
//成员变量std::shared_ptr<ASTVar> var;std::shared_ptr<ASTExpression> expression;    
    need_as_address = true;         /* 设置need_as_address，表示需要返回值 */
    node.var->accept(*this);        /* 处理左var */
    auto var = Res;                 /* 获取地址 */
    node.expression->accept(*this); /* 处理右expression */
    auto res = Res;                 /* 获得结果 */
    auto varType = var->get_type()->get_pointer_element_type(); /* 获取var的类型 */
    /* 若赋值语句左右类型不匹配，则进行匹配 */
    if (varType == FloatType && checkInt(res))
        res = builder->create_sitofp(res, FloatType);
    if (varType == Int32Type && checkFloat(res))
        res = builder->create_fptosi(res, Int32Type);
    builder->create_store(res, var);/* 进行赋值 */
}

/* SimpleExpression, 比较表达式, simple-expression -> additive-expression relop additive-expression
 *                                                  | additive-expression */
void CminusfBuilder::visit(ASTSimpleExpression &node) {  
//std::shared_ptr<ASTAdditiveExpression> additive_expression_l;
//std::shared_ptr<ASTAdditiveExpression> additive_expression_r;
    node.additive_expression_l->accept(*this);  /* 处理左边的expression */
    auto lres = Res;                            /* 获取结果存到lres中 */
    if (node.additive_expression_r == nullptr) { return; }  //* 若不存在右expression，则直接返回 */ 
    node.additive_expression_r->accept(*this);  /* 处理右边的expression */
    auto rres = Res;                            /* 结果存到rres中 */
    if (checkInt(lres) && checkInt(rres)) {     /* 确保两边都是整数 */
        switch (node.op) {  /* 根据不同的比较操作，调用icmp进行处理 */
        /*比较的返回结果
	enum RelOp {
    	// <=
    	OP_LE,
   	 // <
    	OP_LT,
    	// >
    	OP_GT,
    	// >=
    	OP_GE,
    	// ==
    	OP_EQ,
    	// !=
    	OP_NEQ
	};
        */
        case OP_LE:
            Res = builder->create_icmp_le(lres, rres);break;
        case OP_LT:
            Res = builder->create_icmp_lt(lres, rres);break;
        case OP_GT:
            Res = builder->create_icmp_gt(lres, rres);break;
        case OP_GE:
            Res = builder->create_icmp_ge(lres, rres);break;
        case OP_EQ:
            Res = builder->create_icmp_eq(lres, rres);break;
        case OP_NEQ:
            Res = builder->create_icmp_ne(lres, rres);break;
        }
    }
    else {  /* 若有一边是浮点类型，则需要先将另一边也转为浮点数，再进行比较 */
        if (checkInt(lres)) /* 若左边是整数，则将其转为浮点数 */
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) /* 若右边是整数，则将其转为浮点数 */
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {  /* 根据不同的比较操作，调用fcmp进行处理 */
        case OP_LE:
            Res = builder->create_fcmp_le(lres, rres);break;
        case OP_LT:
            Res = builder->create_fcmp_lt(lres, rres);break;
        case OP_GT:
            Res = builder->create_fcmp_gt(lres, rres);break;
        case OP_GE:
            Res = builder->create_fcmp_ge(lres, rres);break;
        case OP_EQ:
            Res = builder->create_fcmp_eq(lres, rres);break;
        case OP_NEQ:
            Res = builder->create_fcmp_ne(lres, rres);break;
        }
    }
    Res = builder->create_zext(Res, Int32Type); /* 将结果作为整数保存（可作为判断语句中的判断条件） */
}

/* AdditiveExpression, 加法表达式, additive-expression -> additive-expression addop term
 *                                                      | term */
void CminusfBuilder::visit(ASTAdditiveExpression &node) {   
    if (node.additive_expression == nullptr) {  /* 若无加减法运算 */
        node.term->accept(*this);return;        /* 则直接去做乘除法 */
    }
    node.additive_expression->accept(*this);    /* 处理左expression */
    auto lres = Res;                            /* 结果保存在lres中 */
    node.term->accept(*this);                   /* 处理右term */
    auto rres = Res;                            /* 结果保存在rres中 */
    if (checkInt(lres) && checkInt(rres)) {     /* 确保两边都是整数 */
        switch (node.op) {  /* 根据对应加法或是减法，调用iadd或是isub进行处理 */
        case OP_PLUS:
            Res = builder->create_iadd(lres, rres);break;
        case OP_MINUS:
            Res = builder->create_isub(lres, rres);break;
        }
    }
    else {  /* 若有一边是浮点类型，则需要先将另一边也转为浮点数，再进行处理 */
        if (checkInt(lres)) /* 若左边是整数，则将其转为浮点数 */
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) /* 若右边是整数，则将其转为浮点数 */
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {  /* 根据对应加法或是减法，调用fadd或是fsub进行处理 */
        case OP_PLUS:
            Res = builder->create_fadd(lres, rres);break;
        case OP_MINUS:
            Res = builder->create_fsub(lres, rres);break;
        }
    }
}

/* Term, 乘除法语句, Term -> term mulop factor
 *                         | factor */
void CminusfBuilder::visit(ASTTerm &node) {     
    if (node.term == nullptr) {             /* 若无乘法运算 */
        node.factor->accept(*this);return;  /* 则直接去处理元素 */
    }
    node.term->accept(*this);   /* 处理左term */
    auto lres = Res;            /* 结果保存在lres中 */
    node.factor->accept(*this); /* 处理右factor */
    auto rres = Res;            /* 结果保存在rres中 */
    if (checkInt(lres) && checkInt(rres)) { /* 确保两边都是整数 */
        switch (node.op) {  /* 根据对应乘法或是除法，调用imul或是idiv进行处理 */
        case OP_MUL:
            Res = builder->create_imul(lres, rres);break;
        case OP_DIV:
            Res = builder->create_isdiv(lres, rres);break;
        }
    }
    else {  /* 若有一边是浮点类型，则需要先将另一边也转为浮点数，再进行处理 */
        if (checkInt(lres)) /* 若左边是整数，则将其转为浮点数 */
            lres = builder->create_sitofp(lres, FloatType);
        if (checkInt(rres)) /* 若右边是整数，则将其转为浮点数 */
            rres = builder->create_sitofp(rres, FloatType);
        switch (node.op) {  /* 根据对应乘法或是除法，调用fmul或是fdiv进行处理 */
        case OP_MUL:
            Res = builder->create_fmul(lres, rres);break;
        case OP_DIV:
            Res = builder->create_fdiv(lres, rres);break;
        }
    }
}

/* Call, 函数调用, call -> ID (args) */
void CminusfBuilder::visit(ASTCall &node) {    
    auto function = static_cast<Function*>(scope.find(node.id));    /* 获取需要调用的函数 */
    auto paramType = function->get_function_type()->param_begin();  /* 获取其参数类型 */
    std::vector<Value*> args;       /* 创建args用于存储函数参数的值，构建调用函数的参数列表 */
    for (auto arg : node.args) {    /* 遍历形参列表 */
        arg->accept(*this);         /* 对每一个参数进行处理，获取参数对应的值 */
        if (Res->get_type()->is_pointer_type()) {   /* 若参数是指针 */
            args.push_back(Res);    /* 则直接将值加入到参数列表 */
        }
        else {  /* 若不是指针，则需要判断形参和实参的类型是否符合。若不符合则需要类型转换 */
            if (*paramType==FloatType && checkInt(Res))
                Res = builder->create_sitofp(Res, FloatType);
            if (*paramType==Int32Type && checkFloat(Res))
                Res = builder->create_fptosi(Res, Int32Type);
            args.push_back(Res);
        }
        paramType++;                /* 查看下一个形参 */
    }
    Res = builder->create_call(static_cast<Function*>(function), args); /* 创建函数调用 */
}