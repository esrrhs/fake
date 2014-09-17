#include "interpreter.h"
#include "fuck.h"
#include "binary.h"
#include "paramstack.h"

void stack::grow(int pos)
{
	if (pos < m_fk->m_stack_ini_size)
	{
		pos = m_fk->m_stack_ini_size;
	}
	assert(m_fk);
	// �¿ռ�
	int newsize = pos + 1 + pos * m_fk->m_stack_grow_speed / 100;
	assert(newsize > (int)m_stack_variant_list_num);
	variant * newbuff = (variant *)m_fk->m_fkmalloc(newsize * sizeof(variant));
	
	// ����
	if (m_stack_variant_list)
	{
		memcpy(newbuff, m_stack_variant_list, m_stack_variant_list_num * sizeof(variant));
	}

	// ����ʣ���
	for (int i = m_stack_variant_list_num; i <newsize; i++)
	{
		new (&newbuff[i]) variant(m_fk);
	}

	// ɾ��
	if (m_stack_variant_list)
	{
		m_fk->m_fkfree(m_stack_variant_list);
	}

	m_stack_variant_list = newbuff;
	m_stack_variant_list_num = newsize;
}

//////////////////////////////////////////////////////////////////////////

void interpreter::grow()
{
	assert(m_fk);
	// �¿ռ�
	int newsize = m_stack_list_max_num + 1 + m_stack_list_max_num * m_fk->m_stack_list_grow_speed / 100;
	assert(newsize > (int)m_stack_list_max_num);
	stack * newbuff = (stack *)m_fk->m_fkmalloc(newsize * sizeof(stack));

	// ����
	if (m_stack_list)
	{
		memcpy(newbuff, m_stack_list, m_stack_list_max_num * sizeof(stack));
	}

	// ����ʣ���
	for (int i = m_stack_list_max_num; i < newsize; i++)
	{
		new (&newbuff[i]) stack();
	}

	// ɾ��
	if (m_stack_list)
	{
		m_fk->m_fkfree(m_stack_list);
	}

	m_stack_list = newbuff;
	m_stack_list_max_num = newsize;
}

