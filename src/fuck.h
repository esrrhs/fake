#pragma once

#include <string>
#include <list>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include <stdio.h>
#include "types.h"
#include "myflexer.h"
#include "fuckscript.h"

struct fkerrorinfo;
struct fuck
{
    fuck()
    {
        clear();
    }

    void clear()
    {
    }
    
    // error
    void seterror(fkerrorinfo * ei, efkerror err, const char *fmt, ...);
    
    // member
    fkmalloc m_fkmalloc;
    fkfree m_fkfree;

    // processorÿ֡����cmd����
    int m_per_frame_cmd_num;
    // processor�����ЧЭ���жϵı���
	int m_delete_routine_scale;
	// Э�������ٶ�
	int m_routine_grow_speed;
	// stack��ʼ��С
	int m_stack_ini_size;
	// stack�����ٶ�
	int m_stack_grow_speed;
	// stack���������ٶ�
	int m_stack_list_grow_speed;
};

template <typename T>
T * fknew(fuck * fk)
{
    T * t = (T *)fk->m_fkmalloc(sizeof(T));
    new (t) T();
    return t;
}

template <typename T, typename P1>
T * fknew(fuck * fk, P1 p1)
{
    T * t = (T *)fk->m_fkmalloc(sizeof(T));
	if (!t)
	{
		return 0;
	}
    new (t) T(p1);
    return t;
}

template <typename T>
void fkdelete(fuck * fk, T * p)
{
	if (p)
	{
		p->~T();
		fk->m_fkfree(p);
	}
}

