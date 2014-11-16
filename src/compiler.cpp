#include "compiler.h"
#include "semantic.h"
#include "myflexer.h"
#include "fake.h"
#include "binary.h"

void compiler::clear()
{
    m_cur_addr = 0;
	m_loop_break_pos_stack.clear();
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
    
    String str = m_fk->bbin.dump();
    FKLOG("[compiler] compile funclist %d ok backup dump \n%s", funclist.size(), str.c_str());

    str = m_fk->bin.dump();
    FKLOG("[compiler] compile funclist %d ok dump \n%s", funclist.size(), str.c_str());

    return true;
}

bool compiler::compile_func(func_desc_node * funcnode)
{
    FKLOG("[compiler] compile_func func %s", funcnode->funcname.c_str());
    
    // �����������ֱ���滻���ϵ�
    codegen cg(m_fk);
    func_binary bin(m_fk);
    
    // ѹջ
    cg.push_stack_identifiers();
        
    // ������ջ
    if (funcnode->arglist)
    {
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
        bin.m_paramnum = arglist.size();
    }
    
    // ���뺯����
    if (funcnode->block)
    {
        if (!compile_block(cg, funcnode->block))
        {
            FKERR("[compile] compile_func compile_block %s fail", funcnode->funcname.c_str());
            return false;
        }
    }
    
    // ����ɹ�
    cg.output(funcnode->funcname.c_str(), &bin);
    m_fk->bbin.add_func(funcnode->funcname.c_str(), bin);
    
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
            if (!compile_block(cg, bn))
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
	case est_for_stmt:
		{
			for_stmt * fs = dynamic_cast<for_stmt *>(node);
			if (!compile_for_stmt(cg, fs))
			{
				FKERR("[compiler] compile_node for_stmt error %d %s", type, node->gettypename());
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
    case est_math_assign_stmt:
        {
            math_assign_stmt * ms = dynamic_cast<math_assign_stmt *>(node);
            if (!compile_math_assign_stmt(cg, ms))
            {
                FKERR("[compiler] compile_node compile_math_assign_stmt error %d %s", type, node->gettypename());
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
            seterror(m_fk, efk_compile_stmt_type_error, "compile node type error %d", type);
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

	int startpos = 0;
	int jnepos = 0;

	m_loop_break_pos_stack.push_back(beak_pos_list());

	startpos = cg.byte_code_size();

	// ����
    cg.push_stack_identifiers();
	if (!compile_node(cg, ws->cmp))
	{
		FKERR("[compiler] compile_while_stmt cmp fail");
		return false;
	}
	cg.pop_stack_identifiers();

	cg.push(MAKE_OPCODE(OPCODE_JNE));
	cg.push(m_cur_addr);
	cg.push(EMPTY_CMD); // ������λ��
	jnepos = cg.byte_code_size() - 1;

	// block��
	if (ws->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, ws->block))
		{
			FKERR("[compiler] compile_while_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// �����жϵط�
	cg.push(MAKE_OPCODE(OPCODE_JMP));
	cg.push(MAKE_POS(startpos));

	// ��ת��block��
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));

	// �滻��break
	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	for (int i = 0; i < (int)bplist.size(); i++)
	{
		cg.set(bplist[i], MAKE_POS(cg.byte_code_size()));
	}
	m_loop_break_pos_stack.pop_back();

    FKLOG("[compiler] compile_while_stmt %p OK", ws);
    
    return true;
}

bool compiler::compile_if_stmt(codegen & cg, if_stmt * is)
{
    FKLOG("[compiler] compile_if_stmt %p", is);

    int jnepos = 0;
    int jmpifpos = 0;

    // ����
    cg.push_stack_identifiers();
	if (!compile_node(cg, is->cmp))
	{
		FKERR("[compiler] compile_if_stmt cmp fail");
		return false;
	}
    cg.pop_stack_identifiers();

	cg.push(MAKE_OPCODE(OPCODE_JNE));
    cg.push(m_cur_addr);
    cg.push(EMPTY_CMD); // ������λ��
    jnepos = cg.byte_code_size() - 1;
    
    // if��
	if (is->block)
    {
        cg.push_stack_identifiers();
    	if (!compile_node(cg, is->block))
    	{
    		FKERR("[compiler] compile_if_stmt block fail");
    		return false;
    	}
        cg.pop_stack_identifiers();

        // ����if��
        cg.push(MAKE_OPCODE(OPCODE_JMP));
        cg.push(EMPTY_CMD); // ������λ��
        jmpifpos = cg.byte_code_size() - 1;
    }
    
    // ��ת��if��
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));
	
	// else��
	if (is->elses && is->elses->block)
	{
        cg.push_stack_identifiers();
	    if (!compile_node(cg, is->elses->block))
    	{
    		FKERR("[compiler] compile_if_stmt else block fail");
    		return false;
    	}
        cg.pop_stack_identifiers();
	}
	
    // ��ת������
	if (jmpifpos != 0)
	{
	    cg.set(jmpifpos, MAKE_POS(cg.byte_code_size()));
	}
    
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

bool compiler::compile_math_assign_stmt(codegen & cg, math_assign_stmt * ms)
{
    FKLOG("[compiler] compile_math_assign_stmt %p", ms);

    command oper = 0;
    command var = 0;
    command value = 0;
    
    if (ms->oper == "+=")
    {
        oper = MAKE_OPCODE(OPCODE_PLUS_ASSIGN);
    }
    else if (ms->oper == "-=")
    {
        oper = MAKE_OPCODE(OPCODE_MINUS_ASSIGN);
    }
    else if (ms->oper == "*=")
    {
        oper = MAKE_OPCODE(OPCODE_MULTIPLY_ASSIGN);
    }
    else if (ms->oper == "/=")
    {
        oper = MAKE_OPCODE(OPCODE_DIVIDE_ASSIGN);
    }
    else if (ms->oper == "%=")
    {
        oper = MAKE_OPCODE(OPCODE_DIVIDE_MOD_ASSIGN);
    }
    else
    {
        FKERR("[compiler] compile_math_assign_stmt error oper type fail");
		seterror(m_fk, efk_compile_math_type_error, "compile math assign oper type %s error", ms->oper.c_str());
        return false;
    }

    // var
    if (!compile_node(cg, ms->var))
    {
        FKERR("[compiler] compile_math_assign_stmt var fail");
        return false;
    }
    var = m_cur_addr;
    FKLOG("[compiler] compile_math_assign_stmt var = %d", m_cur_addr);

    // value
    if (!compile_node(cg, ms->value))
    {
        FKERR("[compiler] compile_math_assign_stmt value fail");
        return false;
    }
    value = m_cur_addr;
    FKLOG("[compiler] compile_math_assign_stmt value = %d", m_cur_addr);

    cg.push(oper);
    cg.push(var);
    cg.push(value);

    FKLOG("[compiler] compile_math_assign_stmt %p OK", ms);
    
    return true;
}

bool compiler::compile_break_stmt(codegen & cg, break_stmt * bs)
{
    FKLOG("[compiler] compile_break_stmt %p", bs);

	cg.push(MAKE_OPCODE(OPCODE_JMP));
	cg.push(EMPTY_CMD); // ������λ��
	int jmppos = cg.byte_code_size() - 1;

	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	bplist.push_back(jmppos);

    FKLOG("[compiler] compile_break_stmt %p OK", bs);
    
    return true;
}

bool compiler::compile_cmp_stmt(codegen & cg, cmp_stmt * cs)
{
    FKLOG("[compiler] compile_cmp_stmt %p", cs);

    command oper = 0;
    command left = 0;
    command right = 0;
    command dest = 0;   

    // oper
    if (cs->cmp == "&&")
    {
        oper = MAKE_OPCODE(OPCODE_AND);
    }
    else if (cs->cmp == "||")
    {
        oper = MAKE_OPCODE(OPCODE_OR);
    }
    else if (cs->cmp == "<")
    {
        oper = MAKE_OPCODE(OPCODE_LESS);
    }
    else if (cs->cmp == ">")
    {
        oper = MAKE_OPCODE(OPCODE_MORE);
    }
    else if (cs->cmp == "==")
    {
        oper = MAKE_OPCODE(OPCODE_EQUAL);
    }
    else if (cs->cmp == ">=")
    {
        oper = MAKE_OPCODE(OPCODE_MOREEQUAL);
    }
    else if (cs->cmp == "<=")
    {
        oper = MAKE_OPCODE(OPCODE_LESSEQUAL);
    }
    else if (cs->cmp == "!=")
    {
        oper = MAKE_OPCODE(OPCODE_NOTEQUAL);
	}
	else if (cs->cmp == "true")
	{
		variant v;
		V_SET_REAL((&v), 1);
		int pos = cg.getconst(v);
		m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);
		return true;
	}
	else if (cs->cmp == "false")
	{
		variant v;
		V_SET_REAL((&v), 0);
		int pos = cg.getconst(v);
		m_cur_addr = MAKE_ADDR(ADDR_CONST, pos);
		return true;
	}
	else
	{
		FKERR("[compiler] compile_cmp_stmt cmp error %s", cs->cmp.c_str());
		return false;
	}

    // left
    if (!compile_node(cg, cs->left))
    {
        FKERR("[compiler] compile_cmp_stmt left fail");
        return false;
    }
    left = m_cur_addr;

    // right
    if (!compile_node(cg, cs->right))
    {
        FKERR("[compiler] compile_cmp_stmt right fail");
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

    FKLOG("[compiler] compile_cmp_stmt %p OK", cs);
    
    return true;
}

bool compiler::compile_explicit_value(codegen & cg, explicit_value_node * ev)
{
	FKLOG("[compiler] compile_explicit_value %p %s", ev, ev->str.c_str());

	fake * fk = m_fk;

	variant v;
	switch (ev->getvaluetype())
	{
	case explicit_value_node::EVT_TRUE:
		V_SET_REAL((&v), 1);
		break;
	case explicit_value_node::EVT_FALSE:
		V_SET_REAL((&v), 0);
		break;
	case explicit_value_node::EVT_NUM:
		V_SET_REAL((&v), (fkatol(&ev->str)));
		break;
	case explicit_value_node::EVT_STR:
		V_SET_STRING((&v), ev->str.c_str());
		break;
	case explicit_value_node::EVT_FLOAT:
		V_SET_REAL((&v), (fkatof(&ev->str)));
		break;
	default:
		FKERR("[compiler] compile_explicit_value type error %d %s", ev->getvaluetype(), ev->gettypename());
		seterror(m_fk, efk_compile_explicit_type_error, "compile explicit value type error %d", ev->getvaluetype());
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

    // �ӵ�ǰ��ջ������
    int pos = cg.getvariable(vn->str);
    if (pos == -1)
    {
        FKERR("[compiler] compile_variable_node variable not found %s", vn->str.c_str());
        seterror(m_fk, efk_compile_variable_not_found, "variable %s not found", vn->str.c_str());
        return false;
    }
    m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

    FKLOG("[compiler] compile_variable_node %p OK", vn);
    
    return true;
}

bool compiler::compile_var_node(codegen & cg, var_node * vn)
{
    FKLOG("[compiler] compile_var_node %p", vn);

	// ȷ����ǰblockû��
	if (cg.get_cur_variable_pos(vn->str) != -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		seterror(m_fk, efk_compile_variable_has_define, "variable %s has define", vn->str.c_str());
		return false;
	}

	// ����ջ�Ͽռ�
	int pos = cg.add_stack_identifier(vn->str);
	if (pos == -1)
	{
		FKERR("[compiler] compile_var_node variable has define %s", vn->str.c_str());
		seterror(m_fk, efk_compile_add_stack_identifier, "add stack variable %s fail", vn->str.c_str());
		return false;
	}
	m_cur_addr = MAKE_ADDR(ADDR_STACK, pos);

    FKLOG("[compiler] compile_var_node %p OK", vn);
    
    return true;
}

bool compiler::compile_function_call_node(codegen & cg, function_call_node * fn)
{
    FKLOG("[compiler] compile_function_call_node %p", fn);

    fake * fk = m_fk;

    // ����
    std::vector<command> arglist;
    if (fn->arglist)
    {
        for (int i = 0; i < (int)fn->arglist->arglist.size(); i++)
        {
            syntree_node* sn = fn->arglist->arglist[i];
            if (!compile_node(cg, sn))
            {
                FKERR("[compiler] compile_function_call_node arg fail");
                return false;
            }
            arglist.push_back(m_cur_addr);
        }
    }

    // ����λ��
    command callpos;
    String func = fn->fuc;
    int pos = cg.getvariable(func);
    if (pos != -1)
    {
        // ���ñ��������ú���
        callpos = MAKE_ADDR(ADDR_STACK, pos);
    }
    else
    {
        // �����ַ�������
	    variant v;
        V_SET_STRING(&v, func.c_str());
        pos = cg.getconst(v);
	    callpos = MAKE_ADDR(ADDR_CONST, pos);
    }

    // oper
    command oper;
    oper = MAKE_OPCODE(OPCODE_CALL);

    // ��������
    command argnum;
    argnum = MAKE_POS(arglist.size());

   
    // ����ֵ
    command ret;
    int retpos = cg.alloc_stack_identifier();
    ret = MAKE_ADDR(ADDR_STACK, retpos);
    m_cur_addr = ret;
    
    cg.push(oper);
    cg.push(callpos);
    cg.push(ret);
    cg.push(argnum);
    for (int i = 0; i < (int)arglist.size(); i++)
    {
        cg.push(arglist[i]);
    }
    
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
		seterror(m_fk, efk_compile_math_type_error, "compile math oper type %s error", mn->oper.c_str());
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

bool compiler::compile_for_stmt(codegen & cg, for_stmt * fs)
{
	FKLOG("[compiler] compile_for_stmt %p", fs);

	int startpos = 0;
	int jnepos = 0;

	m_loop_break_pos_stack.push_back(beak_pos_list());

	// ��ʼ��䣬�����������ȫfor����Ч��
	cg.push_stack_identifiers();
	if (fs->beginblock)
	{
		if (!compile_node(cg, fs->beginblock))
		{
			FKERR("[compiler] compile_for_stmt beginblock fail");
			return false;
		}
	}

	startpos = cg.byte_code_size();

	// ����
	cg.push_stack_identifiers();
	if (!compile_node(cg, fs->cmp))
	{
		FKERR("[compiler] compile_for_stmt cmp fail");
		return false;
	}
	cg.pop_stack_identifiers();

	cg.push(MAKE_OPCODE(OPCODE_JNE));
	cg.push(m_cur_addr);
	cg.push(EMPTY_CMD); // ������λ��
	jnepos = cg.byte_code_size() - 1;

	// block��
	if (fs->block)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, fs->block))
		{
			FKERR("[compiler] compile_for_stmt block fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// ����
	if (fs->endblock)
	{
		cg.push_stack_identifiers();
		if (!compile_node(cg, fs->endblock))
		{
			FKERR("[compiler] compile_for_stmt endblock fail");
			return false;
		}
		cg.pop_stack_identifiers();
	}

	// �����жϵط�
	cg.push(MAKE_OPCODE(OPCODE_JMP));
	cg.push(MAKE_POS(startpos));

	// ��ת��block��
	cg.set(jnepos, MAKE_POS(cg.byte_code_size()));

	// �滻��break
	beak_pos_list & bplist = m_loop_break_pos_stack[m_loop_break_pos_stack.size() - 1];
	for (int i = 0; i < (int)bplist.size(); i++)
	{
		cg.set(bplist[i], MAKE_POS(cg.byte_code_size()));
	}
	m_loop_break_pos_stack.pop_back();

	// �뿪������
	cg.pop_stack_identifiers();

	FKLOG("[compiler] compile_for_stmt %p OK", fs);

	return true;
}


