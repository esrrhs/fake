#pragma once

#include "types.h"
#include "hashmap.h"
#include "buildinfunc.h"
#include "binary.h"
#include "bindfunc.h"

struct funcunion
{
	fkfunctor ff;
	bifunc bif;
	func_binary fb;
	bool haveff;
	bool havebif;
	bool havefb;
};

class funcmap
{
	friend class binary;
	friend class assembler;
public:
	force_inline funcmap(fake * fk) : m_fk(fk), m_shh(fk)
    {
    }

	force_inline ~funcmap()
    {
        clear();
    }

    force_inline fake * getfake()
    {
        return m_fk;
    }

    force_inline void clear()
	{
		for (const vhashmap<funcunion>::ele * p = m_shh.first(); p != 0; p = m_shh.next())
		{
			const funcunion & f = p->t;
			if (f.havefb)
			{
				FUNC_BINARY_DELETE(f.fb);
			}
		}
        m_shh.clear();
    }

	force_inline const funcunion * get_func(const variant & name) const
	{
		return m_shh.get(name);
	}

	force_inline void add_func(const variant & name, const func_binary & fb)
	{
		funcunion *  f = add_func_union(name);
		if (f->havefb)
		{
			FUNC_BINARY_DELETE(f->fb);
		}
		f->fb = fb;
		f->havefb = true;
	}

	force_inline void add_bind_func(const variant & name, const fkfunctor & ff)
	{
		funcunion *  f = add_func_union(name);
		f->ff = ff;
		f->haveff = true;
	}

	force_inline void add_buildin_func(const variant & name, const bifunc & bif)
	{
		funcunion *  f = add_func_union(name);
		f->bif = bif;
		f->havebif = true;
	}

private:
	force_inline funcunion * add_func_union(const variant & name)
	{
		funcunion * p = m_shh.get(name);
		if (p)
		{
			return p;
		}

		funcunion tmp;
		memset(&tmp, 0, sizeof(tmp));
		return &m_shh.add(name, tmp)->t;
	}

private:
    fake * m_fk;
	vhashmap<funcunion> m_shh;
};
