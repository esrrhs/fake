#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void stack::grow(int pos)
{
	if (pos < m_fk->cfg.stack_ini_size)
	{
		pos = m_fk->cfg.stack_ini_size;
	}
	assert(m_fk);
	// �¿ռ�
	int newsize = pos + 1 + pos * m_fk->cfg.stack_grow_speed / 100;
	assert(newsize > (int)m_stack_variant_list_num);
	variant * newbuff = (variant *)safe_fkmalloc(m_fk, (newsize * sizeof(variant)));
	
	// ����
	if (m_stack_variant_list)
	{
		memcpy(newbuff, m_stack_variant_list, m_stack_variant_list_num * sizeof(variant));
	}

	// ����ʣ���
	memset(&newbuff[m_stack_variant_list_num], 0, (newsize - m_stack_variant_list_num) * sizeof(variant));

	// ɾ��
	safe_fkfree(m_fk, m_stack_variant_list);

	m_stack_variant_list = newbuff;
	m_stack_variant_list_num = newsize;
}

//////////////////////////////////////////////////////////////////////////

void interpreter::grow()
{
	assert(m_fk);
	// �¿ռ�
	int newsize = m_stack_list_max_num + 1 + m_stack_list_max_num * m_fk->cfg.stack_list_grow_speed / 100;
	assert(newsize > (int)m_stack_list_max_num);
	stack * newbuff = (stack *)safe_fkmalloc(m_fk, (newsize * sizeof(stack)));

	// ����
	if (m_stack_list)
	{
		memcpy(newbuff, m_stack_list, m_stack_list_max_num * sizeof(stack));
	}

	// ����ʣ���
	memset(&newbuff[m_stack_list_max_num], 0, (newsize - m_stack_list_max_num) * sizeof(stack));

	// ɾ��
	safe_fkfree(m_fk, m_stack_list);

	m_stack_list = newbuff;
	m_stack_list_max_num = newsize;
}

