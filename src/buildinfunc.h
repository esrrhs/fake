#pragma once

#include "types.h"
#include "hashmap.h"

typedef void (*bifunc)(fake * fk);

class buildinfunc
{
public:
    force_inline buildinfunc(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }
    force_inline ~buildinfunc()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
    {
        m_shh.clear();
    }

    void openbasefunc();
    
    // �����ͷ���ֵ����m_fk->ps��
    bool call(const char * name);
    
private:
    fake * m_fk;
    stringhashmap<bifunc> m_shh;
};

