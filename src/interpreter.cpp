#include "interpreter.h"
#include "fake.h"
#include "binary.h"
#include "paramstack.h"

//////////////////////////////////////////////////////////////////////////

void interpreter::call(binary * bin, const variant & func, paramstack * ps)
{
	const funcunion * f = m_fk->fm.get_func(func);
	if (!f)
	{
		FKERR("fkrun bin %p no func %s fail", bin, vartostring(&func).c_str());
		seterror(m_fk, efk_run_no_func_error, "fkrun bin %p no func %s fail", bin, vartostring(&func).c_str());
		m_isend = true;
	}

	// ���溯��
	if (f->havefb)
	{
		const func_binary * fb = &f->fb;

		// �պ�������
		if (!FUNC_BINARY_CMDSIZE(*fb))
		{
			// ���ж���
			if (ARRAY_EMPTY(m_stack_list))
			{
				FKLOG("call stack empty end");
				m_isend = true;
			}
			return;
		}

		// ѹջ
		if (ARRAY_SIZE(m_stack_list) >= ARRAY_MAX_SIZE(m_stack_list))
		{
			int newsize = ARRAY_MAX_SIZE(m_stack_list) + 1 + ARRAY_MAX_SIZE(m_stack_list) * m_fk->cfg.array_grow_speed / 100;
			ARRAY_GROW(m_stack_list, newsize, stack);
		}
		ARRAY_PUSH_BACK(m_stack_list);
		stack & s = ARRAY_BACK(m_stack_list);
		m_cur_stack = &s;
		STACK_INI(s, m_fk, fb);
		if (FUNC_BINARY_MAX_STACK(*fb) > (int)ARRAY_MAX_SIZE(s.m_stack_variant_list))
		{
			ARRAY_GROW(s.m_stack_variant_list, FUNC_BINARY_MAX_STACK(*fb), variant);
		}

		// ��¼profile
		beginfuncprofile();

		// ����ջ�ռ�
		for (int i = 0; i < (int)ps->m_variant_list_num; i++)
		{
			SET_STACK(&(ps->m_variant_list[i]), s, i);
			FKLOG("call set %s to pos %d", (vartostring(&(ps->m_variant_list[i]))).c_str(), i);
		}
		ps->clear();

		// ����ret
		V_SET_NIL(&m_ret[0]);

		return;
	}

	// ��¼profile
	uint32_t s = 0;
	if (m_fk->pf.isopen())
	{
		s = fkgetmstick();
	}

	// �󶨺���
	if (f->haveff)
	{
		BIND_FUNC_CALL(f, this);
		FKLOG("call C func %s", vartostring(&func).c_str());
	}
	// ���ú���
	else if (f->havebif)
	{
		BUILDIN_FUNC_CALL(f, this);
		FKLOG("call buildin func %s", vartostring(&func).c_str());
	}
	else
	{
		assert(0);
	}

	// ����ֵ
	paramstack * theps = getps(m_fk);
	variant * cret;
	PS_POP_AND_GET(*theps, cret);

	// ���������ֱ�������ű�������C����
	if (ARRAY_EMPTY(m_stack_list))
	{
		m_isend = true;
		// ֱ��������ֵ
		m_ret[0] = *cret;
	}
	// ����������ǰ��ջ��
	else
	{
		// ������ֵ
		bool err = false;
		m_cur_stack = &ARRAY_BACK(m_stack_list);
		const func_binary & fb = *m_cur_stack->m_fb;
		variant * ret;
		do { GET_VARIANT(*m_cur_stack, fb, ret, m_cur_stack->m_retvpos[0]); } while (0);

		*ret = *cret;
	}

	if (m_fk->pf.isopen())
	{
		const char * name = 0;
		V_GET_STRING(&func, name);
		m_fk->pf.add_func_sample(name, fkgetmstick() - s);
	}

	return;
}

void interpreter::call(fake * fk, const variant & callpos, paramstack * ps, int calltype)
{
    if (calltype == CALL_NORMAL)
    {
        call(&m_fk->bin, callpos, ps);
    }
    else
    {
		m_processor->start_routine(&m_fk->bin, callpos, ps);
    }
}

void interpreter::beginfuncprofile()
{
    if (m_fk->pf.isopen())
    {
        m_cur_stack->m_calltime = fkgetmstick();
    }
}

void interpreter::endfuncprofile()
{
    if (m_fk->pf.isopen())
    {
		m_fk->pf.add_func_sample(FUNC_BINARY_NAME(*m_cur_stack->m_fb), fkgetmstick() - m_cur_stack->m_calltime);
    }
}

