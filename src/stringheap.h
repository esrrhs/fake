#pragma once

#include "types.h"
#include "hashmap.h"

struct fuck;

struct stringele
{
    // ���¼���
    uint32_t update;
	// string
    char * s;
    size_t sz;
};

class stringheap
{
public:
    stringheap(fuck * fk);
	~stringheap();

	void clear();

	stringele * allocstring(const char * str);

private:
	void gc();

private:
    fuck * m_fk;    
    stringhashmap<stringele> m_shh;
};

