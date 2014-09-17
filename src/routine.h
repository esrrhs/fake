#pragma once

#include "types.h"
#include "variant.h"
#include "interpreter.h"

struct fuck;
class fkerrorinfo;
class binary;
class paramstack;
class routine
{
public:
    force_inline routine(fuck * fk, fkerrorinfo * ei) : m_fk(fk), m_ei(ei), m_bin(0), m_ps(0), m_interpreter(fk, ei)
    {
    }

    force_inline void entry(binary * bin, const String & func, paramstack * ps)
    {
        m_bin = bin;
        m_func = func;
        m_ps = ps;
        m_interpreter.call(m_bin, m_func, m_ps);
    }
    
    force_inline const variant & getret() const
    {
        return m_ret;
    }


    force_inline bool isend() const
    {
        return m_interpreter.isend();
    }
    
    force_inline int run(int cmdnum)
    {
        int runcmdnum = m_interpreter.run(cmdnum);

        if (isend())
        {   
            m_ret = m_interpreter.getret();
        }

        return runcmdnum;
    }    


private:
    fuck * m_fk;
    fkerrorinfo * m_ei;
    binary * m_bin;
    String m_func;
    paramstack * m_ps;
    variant m_ret;
    // ������
    interpreter m_interpreter;
};

