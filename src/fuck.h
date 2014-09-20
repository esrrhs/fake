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
#include "compiler.h"
#include "binary.h"
#include "paramstack.h"
#include "interpreter.h"
#include "native.h"
#include "assembler.h"
#include "machine.h"

struct fuck
{
    fuck() : errorno(0), mf(this), bin(this), mc(this, &bin), nt(this), as(this, &nt), inter(this), mac(this)
    {
    }
    ~fuck()
    {
    }

    // ���
    void clear()
    {
        clearerr();
        mf.clear();
        bin.clear();
        mc.clear();
        nt.clear();
        as.clear();
        ps.clear();
        inter.clear();
        mac.clear();
    }
    
    void clearerr()
    {
        errorno = 0;
        errorstr[0] = 0;
    }
    
    int errorno;
    char errorstr[64];
    
    // ����
    fuckconfig cfg;

    // ����
    myflexer mf;

    // ������
    binary bin;
    
    // ����
    compiler mc;

    // ����jit����
    native nt;

    // �����
    assembler as;

    // c�Ĳ���ջ
    paramstack ps;

    // ��ǰ�߳̽�����
    interpreter inter;

    // ��ǰ�̱߳��ػ���
    machine mac;

    // TODO �첽�̵߳����л���
};

template <typename T>
T * fknew(fuck * fk)
{
    T * t = (T *)fk->cfg.fkm(sizeof(T));
    new (t) T();
    return t;
}

template <typename T, typename P1>
T * fknew(fuck * fk, P1 p1)
{
    T * t = (T *)fk->cfg.fkm(sizeof(T));
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
		fk->cfg.fkf(p);
	}
}

