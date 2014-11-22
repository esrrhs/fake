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
#include "fakescript.h"
#include "compiler.h"
#include "binary.h"
#include "paramstack.h"
#include "interpreter.h"
#include "native.h"
#include "assembler.h"
#include "machine.h"
#include "stringheap.h"
#include "bindfunc.h"
#include "buildinfunc.h"
#include "profile.h"
#include "pool.h"
#include "processor.h"
#include "container.h"
#include "funcmap.h"

struct fake
{
	fake() : errorno(0), mf(this), bbin(this), bin(this), mc(this), nt(this), as(this, &nt), sh(this), mac(this), bf(this), bif(this), pf(this), rundeps(0), con(this), fm(this)
    {
        POOL_INI(pp, this);
    }
    ~fake()
    {
        clear();
        POOL_DELETE(pp);
    }

    // ���
    void clear()
    {
        clearerr();
        mf.clear();
        bbin.clear();
        bin.clear();
        mc.clear();
        nt.clear();
        as.clear();
        ps.clear();
        POOL_CLEAR(pp);
        sh.clear();
        mac.clear();
        bf.clear();
        bif.clear();
        pf.clear();
		rundeps = 0;
		con.clear();
		fm.clear();
    }
    
    void clearerr()
    {
        errorno = 0;
        errorstr[0] = 0;
    }
    
    int errorno;
    char errorstr[128];
    
    // ����
    fakeconfig cfg;

    // ����
    myflexer mf;

    // ���������
    backupbinary bbin;

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

    // ִ��������
    pool<processor> pp;

    // �ַ�������
    stringheap sh;

    // ��ǰ�̱߳��ػ���
    machine mac;

    // ��C��������
    bindfunc bf;

    // �ڽ��ĺ�������
    buildinfunc bif;

    // ���ܼ��
    profile pf;

    // ִ�е�������
    int rundeps;

    // ����
    container con;

	// ��������
	funcmap fm;
};

template <typename T>
T * fknew(fake * fk)
{
    T * t = (T *)fk->cfg.fkm(sizeof(T));
    new (t) T();
    return t;
}

template <typename T, typename P1>
T * fknew(fake * fk, P1 p1)
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
void fkdelete(fake * fk, T * p)
{
	if (p)
	{
		p->~T();
		fk->cfg.fkf(p);
	}
}

