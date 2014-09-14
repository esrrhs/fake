#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void interpreter::clear()
{
    m_isend = false;
    m_ret = variant(m_fk);
    m_stack_list.clear();
}

bool interpreter::isend() const
{
    return m_isend;
}

const variant & interpreter::getret() const
{
    return m_ret;
}

void interpreter::call(binary * bin, const String & func, paramstack * ps)
{
    const func_binary * fb = bin->get_func(func);
    if (!fb)
    {
        FKERR("fkrun bin %p no func %s fail", bin, func.c_str());
        m_fk->seterror(m_ei, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, func.c_str());
        m_isend = true;
        return;
    }

    // ѹջ
    m_stack_list.push_back(stack(m_fk, m_ei, fb));
    stack & s = m_stack_list.back();

    // ����ջ�ռ�
    for (int i = 0; i < (int)ps->size(); i++)
    {
		s.set_stack_variant((*ps)[i], i);
		FKLOG("call set %s to pos %d", ((String)((*ps)[i])).c_str(), i);
    }
}

int interpreter::run(int cmdnum)
{
    int num = 0;
    for (int i = 0; i < cmdnum; i++)
    {
        if (!next())
        {
            // ��������
            m_isend = true;
        }
        num++;
        if (isend())
        {
            break;
        }
    }

    return num;
}

bool interpreter::next()
{
    stack & s = m_stack_list.back();
    const func_binary & fb = *s.m_fb;
    int pos = s.m_pos;

    command cmd = fb.getcmd(pos);
    int type = COMMAND_TYPE(cmd);
    int code = COMMAND_CODE(cmd);

    FKLOG("next %d %d %s", type, code, OpCodeStr(code));
        
    assert (type == COMMAND_OPCODE);

    s.m_pos++;

    bool ret = false;
    switch (code)
    {
    case OPCODE_ASSIGN:
        ret = next_assign(s, fb, code);
        break;
    case OPCODE_PLUS:
    case OPCODE_MINUS:
    case OPCODE_MULTIPLY:
    case OPCODE_DIVIDE:
    case OPCODE_DIVIDE_MOD:
        ret = next_math(s, fb, code);
		break;
	case OPCODE_RETURN:
		ret = next_return(s, fb, code);
		break;
    default:
        assert(0);
        FKERR("next err code %d %s", code, OpCodeStr(code));
        break;
    }

    // ��ǰ��������
    if (s.m_pos >= (int)fb.cmdsize())
    {
        FKLOG("pop stack %s", s.m_fb->getname().c_str());
        // ��ջ
        m_stack_list.pop_back();
    }

    // ���ж���
    if (m_stack_list.empty())
    {
        FKLOG("stack empty end");
        m_isend = true;
    }

    return ret;
}

#define GET_VARIANT(s, fb, v, pos) \
    command v##_cmd = fb.getcmd(pos);\
    assert (COMMAND_TYPE(v##_cmd) == COMMAND_ADDR);\
    int v##_addrtype = ADDR_TYPE(COMMAND_CODE(v##_cmd));\
    int v##_addrpos = ADDR_POS(COMMAND_CODE(v##_cmd));\
    assert (v##_addrtype == ADDR_STACK || v##_addrtype == ADDR_CONST);\
    if (v##_addrtype == ADDR_STACK)\
    {\
        s.get_stack_variant(v, v##_addrpos);\
    }\
    else if (v##_addrtype == ADDR_CONST)\
    {\
        fb.getconst(v, v##_addrpos);\
    }\
    else\
    {\
        assert(0);\
        FKERR("next_assign assignaddrtype cannot be %d %d", v##_addrtype, v##_addrpos);\
        return false;\
    }

#define LOG_VARIANT(s, fb, pos, prefix) \
    FKLOG(prefix " variant %d %d", \
        ADDR_TYPE(COMMAND_CODE(fb.getcmd(pos))),\
        ADDR_POS(COMMAND_CODE(fb.getcmd(pos))));

bool interpreter::next_assign(stack & s, const func_binary & fb, int code)
{
    // ��ֵdest������Ϊջ��
    assert (ADDR_TYPE(COMMAND_CODE(fb.getcmd(s.m_pos))) == ADDR_STACK);
    LOG_VARIANT(s, fb, s.m_pos, "assign");
    int addrpos = ADDR_POS(COMMAND_CODE(fb.getcmd(s.m_pos)));
    s.m_pos++;

    // ��ֵ��Դ
    variant valuev(m_fk);
    LOG_VARIANT(s, fb, s.m_pos, "value");
    GET_VARIANT(s, fb, valuev, s.m_pos);
    s.m_pos++;

    // ��ֵ
    s.set_stack_variant(valuev, addrpos);

    FKLOG("assign %s to pos %d", ((String)valuev).c_str(), addrpos);
    
    return true;
}

bool interpreter::next_math(stack & s, const func_binary & fb, int code)
{
	variant left(m_fk);
	LOG_VARIANT(s, fb, s.m_pos, "left");
    GET_VARIANT(s, fb, left, s.m_pos);
    s.m_pos++;
    
	variant right(m_fk);
	LOG_VARIANT(s, fb, s.m_pos, "right");
    GET_VARIANT(s, fb, right, s.m_pos);
    s.m_pos++;
    
    assert (ADDR_TYPE(COMMAND_CODE(fb.getcmd(s.m_pos))) == ADDR_STACK);
    LOG_VARIANT(s, fb, s.m_pos, "math");
    int addrpos = ADDR_POS(COMMAND_CODE(fb.getcmd(s.m_pos)));
    s.m_pos++;

	FKLOG("math left %s right %s", ((String)left).c_str(), ((String)right).c_str());

    switch (code)
    {
    case OPCODE_PLUS:
        left += right;
        break;
    case OPCODE_MINUS:
        left -= right;
        break;
    case OPCODE_MULTIPLY:
        left *= right;
        break;
    case OPCODE_DIVIDE:
        left /= right;
        break;
    case OPCODE_DIVIDE_MOD:
        left %= right;
        break;
    default:
        assert(0);
        FKERR("math err code %d %s", code, OpCodeStr(code));
        break;
    }

    s.set_stack_variant(left, addrpos);

    FKLOG("math %s %s to pos %d", OpCodeStr(code), ((String)left).c_str(), addrpos);
    
    return true;
}

bool interpreter::next_return(stack & s, const func_binary & fb, int code)
{
	if (fb.getcmd(s.m_pos) == EMPTY_CMD)
	{
		FKLOG("return empty");
		return true;
	}

	// ����ret
	LOG_VARIANT(s, fb, s.m_pos, "ret");
	GET_VARIANT(s, fb, m_ret, s.m_pos);
	s.m_pos++;

	FKLOG("return %s", ((String)m_ret).c_str());

	return true;
}

