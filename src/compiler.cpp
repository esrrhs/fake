#include "compiler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fuck.h"
#include "binary.h"

void compiler::clear()
{
    m_binary->clear();
}

bool compiler::compile(myflexer * mf)
{
    func_desc_list & funclist = mf->get_func_list();
    FKLOG("[compiler] compile funclist %d", funclist.size());

    for (int i = 0; i < (int)funclist.size(); i++)
    {
        func_desc_node * funcnode = funclist[i];
        if (!compile_func(funcnode))
        {   
            FKERR("[compiler] compile %s fail", funcnode->funcname.c_str());
            return false;
        }
    }

    String str = m_binary->dump();
    FKLOG("[compiler] compile funclist %d ok dump \n%s", funclist.size(), str.c_str());

    return true;
}

bool compiler::compile_func(func_desc_node * funcnode)
{
    FKLOG("[compiler] compile_func func %s", funcnode->funcname.c_str());
    
    codegen cg(m_fk, m_ei);
    
    // 检测重名
    if (m_binary->is_have_func(funcnode->funcname))
    {
        m_fk->seterror(m_ei, efk_compile_same_func_name, "same func name %s", funcnode->funcname.c_str());
        return false;
    }

    // 参数入栈
    func_desc_arglist & arglist = funcnode->arglist->arglist;
    for (int i = 0; i < (int)arglist.size(); i++)
    {
        const String & arg = arglist[i];
        if (cg.add_stack_identifier(arg) == -1)
        {
            FKERR("[compile] compile_func %s arg error %s", funcnode->funcname.c_str(), arg.c_str());
            return false;
        }
    }

    // 编译函数体
    if (!compile_block(cg, funcnode->block))
    {
        FKERR("[compile] compile_func compile_block %s fail", funcnode->funcname.c_str());
        return false;
    }

    // 编译成功
    func_binary bin(m_fk);
    cg.output(funcnode->funcname, &bin);
    m_binary->add_func(bin);
    
    FKLOG("[compiler] compile_func func %s size = %d OK", funcnode->funcname.c_str(), bin.size());
    
    return true;
}

bool compiler::compile_block(codegen & cg, block_node * blocknode)
{
    FKLOG("[compiler] compile_block block_node %p", blocknode);

    for (int i = 0; i < (int)blocknode->stmtlist.size(); i++)
    {
        syntree_node * stmt = blocknode->stmtlist[i];
        if (!compile_node(cg, stmt))
        {
            FKERR("[compiler] compile_block compile_node %p fail %s", blocknode, stmt->gettypename());
            return false;
        }
    }
    
    FKLOG("[compiler] compile_block block_node %p OK", blocknode);
    
    return true;
}

bool compiler::compile_node(codegen & cg, syntree_node * node)
{
    FKLOG("[compiler] compile_node %p %s", node, node->gettypename());

    esyntreetype type = node->gettype();
    switch (type)
    {
    case est_block:
        {
            block_node * bn = dynamic_cast<block_node *>(node);
            if (!compile_block(cg, bn + 1))
            {
                FKERR("[compiler] compile_node block_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_while_stmt:
        {
            while_stmt * ws = dynamic_cast<while_stmt *>(node);
            if (!compile_while_stmt(cg, ws))
            {
                FKERR("[compiler] compile_node while_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_cmp_stmt:
        {
            cmp_stmt * cs = dynamic_cast<cmp_stmt *>(node);
            if (!compile_cmp_stmt(cg, cs))
            {
                FKERR("[compiler] compile_node cmp_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_if_stmt:
        {
            if_stmt * is = dynamic_cast<if_stmt *>(node);
            if (!compile_if_stmt(cg, is))
            {
                FKERR("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_explicit_value:
        {
            explicit_value_node * ev = dynamic_cast<explicit_value_node *>(node);
            if (!compile_explicit_value(cg, ev))
            {
                FKERR("[compiler] compile_node compile_if_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_return_stmt:
        {
            return_stmt * rs = dynamic_cast<return_stmt *>(node);
            if (!compile_return_stmt(cg, rs))
            {
                FKERR("[compiler] compile_node compile_return_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_assign_stmt:
        {
            assign_stmt * as = dynamic_cast<assign_stmt *>(node);
            if (!compile_assign_stmt(cg, as))
            {
                FKERR("[compiler] compile_node compile_assign_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_variable:
        {
            variable_node * vn = dynamic_cast<variable_node *>(node);
            if (!compile_variable_node(cg, vn))
            {
                FKERR("[compiler] compile_node compile_variable_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_var:
        {
            var_node * vn = dynamic_cast<var_node *>(node);
            if (!compile_var_node(cg, vn))
            {
                FKERR("[compiler] compile_node compile_var_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_function_call:
        {
            function_call_node * fn = dynamic_cast<function_call_node *>(node);
            if (!compile_function_call_node(cg, fn))
            {
                FKERR("[compiler] compile_node function_call_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_break:
        {
            break_stmt * bs = dynamic_cast<break_stmt *>(node);
            if (!compile_break_stmt(cg, bs))
            {
                FKERR("[compiler] compile_node compile_break_stmt error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_math_expr:
        {
            math_expr_node * mn = dynamic_cast<math_expr_node *>(node);
            if (!compile_math_expr_node(cg, mn))
            {
                FKERR("[compiler] compile_node math_expr_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    case est_identifier:
        {
            identifier_node * in = dynamic_cast<identifier_node *>(node);
            if (!compile_identifier_node(cg, in))
            {
                FKERR("[compiler] compile_node compile_identifier_node error %d %s", type, node->gettypename());
                return false;
            }
        }
        break;
    default:
        {
            FKERR("[compiler] compile_node type error %d %s", type, node->gettypename());
            m_fk->seterror(m_ei, efk_compile_stmt_type_error, "compile node type error %d", type);
            return false;
        }
        break;
    }

    FKLOG("[compiler] compile_node %p %s OK", node, node->gettypename());
    
    return true;
}

bool compiler::compile_while_stmt(codegen & cg, while_stmt * ws)
{
    FKLOG("[compiler] compile_while_stmt %p", ws);

    FKLOG("[compiler] compile_while_stmt %p OK", ws);
    
    return true;
}

bool compiler::compile_if_stmt(codegen & cg, if_stmt * is)
{
    FKLOG("[compiler] compile_if_stmt %p", is);

    FKLOG("[compiler] compile_if_stmt %p OK", is);
    
    return true;
}

bool compiler::compile_return_stmt(codegen & cg, return_stmt * rs)
{
    FKLOG("[compiler] compile_return_stmt %p", rs);

	if (rs->ret)
	{
		if (!compile_node(cg, rs->ret))
		{
			FKERR("[compiler] compile_return_stmt ret fail");
			return false;
		}
		cg.push(MAKE_OPCODE(OPCODE_RETURN));
		cg.push(m_cur_addr);
	}
	else
	{
		cg.push(MAKE_OPCODE(OPCODE_RETURN));
		cg.push(EMPTY_CMD);
	}

    FKLOG("[compiler] compile_return_stmt %p OK", rs);
    
    return true;
}

bool compiler::compile_assign_stmt(codegen & cg, assign_stmt * as)
{
    FKLOG("[compiler] compile_assign_stmt %p", as);

    command var = 0;
    command value = 0;

    if (!compile_node(cg, as->var))
    {
        FKERR("[compiler] compile_assign_stmt var fail");
        return false;
    }
    var = m_cur_addr;
    FKLOG("[compiler] compile_assign_stmt var = %d", m_cur_addr);
    
    if (!compile_node(cg, as->value))
    {
        FKERR("[compiler] compile_assign_stmt value fail");
        return false;
    }
    value = m_cur_addr;
    FKLOG("[compiler] compile_assign_stmt value = %d", m_cur_addr);

    cg.push(MAKE_OPCODE(OPCODE_ASSIGN));
    cg.push(var);
    cg.push(value);
    FKLOG("[compiler] compile_assign_stmt %p OK", as);
    
    return true;
}

bool compiler::compile_break_stmt(codegen & cg, break_stmt * bs)
{
    FKLOG("[compiler] compile_break_stmt %p", bs);

    FKLOG("[compiler] compile_break_stmt %p OK", bs);
    
    return true;
}

bool compiler::compile_cmp_stmt(codegen & cg, cmp_stmt * cs)
{
    FKLOG("[compiler] compile_cmp_stmt %p", cs);

    FKLOG("[compiler] compile_cmp_stmt %p OK", cs);
    
    return true;
}

bool compiler::compile_explicit_value(codegen & cg, explicit_value_node * ev)
{
	FKLOG("[compiler] compile_explicit_value %p %s", ev, ev->str.c_str());

	variant v(m_fk);
	switch (ev->getvaluetype())
	{
	case explicit_value_node::EVT_TRUE:
		v = variant(1, m_fk);
		break;
	case explicit_value_node::EVT_FALSE:
		v = variant(0, m_fk);
		break;
	case explicit_value_node::EVT_NUM:
		v = variant(fkatol(&ev->str), m_fk);
		break;
	case explicit_value_node::EVT_STR:
		v = variant(ev->str, m_fk);
		break;
	case explicit_value_node::EVT_FLOAT:
		v = variant(fkatof(&ev->str), m_fk);
		break;
	default:
		FKERR("[compiler] compile_explicit_value type error %d %s", ev->getvaluetype(), ev->gettypename());
		m_fk->seterror(m_ei, efk_compile_explicit_type_error, "compile explicit value type error %d", ev->getvaluetype());
		return false;
	}

	int pos = cg.getconst(v);
	m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);

    FKLOG("[compiler] compile_explicit_value %p OK", ev);
    
    return true;
}

bool compiler::compile_variable_node(codegen & cg, variable_node * vn)
{
    FKLOG("[compiler] compile_variable_node %p", vn);

    // 从当前堆栈往上找
    int pos = cg.getvariable(vn->str);
    if (pos == -1)
    {
        FKERR("[compiler] compile_variable_node variable not found %s", vn->str.c_str());
        m_fk->seterror(m_ei, efk_compile_variable_not_found, "variable %s not found", vn->str.c_str());
        return false;
    }
    m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

    FKLOG("[compiler] compile_variable_node %p OK", vn);
    
    return true;
}

bool compiler::compile_var_node(codegen & cg, var_node * vn)
{
    FKLOG("[compiler] compile_var_node %p", vn);

	// 确保当前block没有
	if (cg.get_cur_variable_pos(vn->str) != -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		m_fk->seterror(m_ei, efk_compile_variable_has_define, "variable %s has define", vn->str.c_str());
		return false;
	}

	// 申请栈上空间
	int pos = cg.add_stack_identifier(vn->str);
	if (pos == -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		m_fk->seterror(m_ei, efk_compile_add_stack_identifier, "add stack variable %s fail", vn->str.c_str());
		return false;
	}
	m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

    FKLOG("[compiler] compile_var_node %p OK", vn);
    
    return true;
}

bool compiler::compile_function_call_node(codegen & cg, function_call_node * fn)
{
    FKLOG("[compiler] compile_function_call_node %p", fn);

    FKLOG("[compiler] compile_function_call_node %p OK", fn);
    
    return true;
}

bool compiler::compile_math_expr_node(codegen & cg, math_expr_node * mn)
{
    FKLOG("[compiler] compile_math_expr_node %p", mn);

    command oper = 0;
    command left = 0;
    command right = 0;
    command dest = 0;
    
    if (mn->oper == "+")
    {
        oper = MAKE_OPCODE(OPCODE_PLUS);
    }
    else if (mn->oper == "-")
    {
        oper = MAKE_OPCODE(OPCODE_MINUS);
    }
    else if (mn->oper == "*")
    {
        oper = MAKE_OPCODE(OPCODE_MULTIPLY);
    }
    else if (mn->oper == "/")
    {
        oper = MAKE_OPCODE(OPCODE_DIVIDE);
    }
    else if (mn->oper == "%")
    {
        oper = MAKE_OPCODE(OPCODE_DIVIDE_MOD);
    }
    else
    {
        FKERR("[compiler] compile_math_expr_node error oper type fail");
		m_fk->seterror(m_ei, efk_compile_math_type_error, "compile math oper type %s error", mn->oper.c_str());
        return false;
    }

    // left
    if (!compile_node(cg, mn->left))
    {
        FKERR("[compiler] compile_math_expr_node left fail");
        return false;
    }
    left = m_cur_addr;

    // right
    if (!compile_node(cg, mn->right))
    {
        FKERR("[compiler] compile_math_expr_node left fail");
        return false;
    }
    right = m_cur_addr;

    // result
    int despos = cg.alloc_stack_identifier();
    dest = MAKE_ADDR(ADDR_STACK, despos);
    m_cur_addr = dest;
    
    cg.push(oper);
    cg.push(left);
    cg.push(right);
    cg.push(dest);

    FKLOG("[compiler] compile_math_expr_node %p OK", mn);
    
    return true;
}

bool compiler::compile_identifier_node(codegen & cg, identifier_node * in)
{
    FKLOG("[compiler] compile_identifier_node %p", in);

    FKLOG("[compiler] compile_identifier_node %p OK", in);
    
    return true;
}


