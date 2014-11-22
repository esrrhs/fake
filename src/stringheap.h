#pragma once

#include "types.h"
#include "hashmap.h"
#include "array.h"

struct fake;

struct stringele
{
    // ���¼���
    uint32_t update;
	// string
    char * s;
    size_t sz;
	// ϵͳ����
	uint32_t sysref;
};

class stringheap
{
public:
    stringheap(fake * fk);
	~stringheap();

	void clear();

	stringele * allocstring(const char * str);

	variant allocsysstr(const char * str);

	void checkgc();
	
private:
	void gc();

private:
	fake * m_fk;
	stringhashmap<stringele*> m_shh;
	array<stringele*> m_sysstr_list;
};

