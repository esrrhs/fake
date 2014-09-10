#include "fuckscript.h"
#include "semantic.h"
#include "fuck.h"
#include "compiler.h"
#include "routine.h"
#include "processor.h"
#include "paramstack.h"

fuck * newfuck(fkmalloc fkm, fkfree fkf,
    int per_frame_cmd_num,
    int delete_routine_scale)
{
    if (!fkm || !fkf)
    {
        fkm = &malloc;
        fkf = &free;
        FKLOG("newfuck use system malloc and free");
    }
    if (!per_frame_cmd_num || !delete_routine_scale)
    {
        per_frame_cmd_num = 100;
        delete_routine_scale = 4;
        FKLOG("newfuck use per_frame_cmd_num[%d] and delete_routine_scale[%d]", per_frame_cmd_num, delete_routine_scale);
    }
    
    fuck * ret = (fuck *)fkm(sizeof(fuck));
    new (ret) fuck();
    ret->m_fkmalloc = fkm;
    ret->m_fkfree = fkf;
    ret->m_per_frame_cmd_num = per_frame_cmd_num;
    ret->m_delete_routine_scale = delete_routine_scale;
    FKLOG("newfuck ret %p", ret);
    return ret;
}

void delfuck(fuck * fk)
{
    FKLOG("delfuck %p", fk);
    fk->~fuck();
    fk->m_fkfree(fk);
}

// �����ļ�
binary * fkparse(fuck * fk, fkerrorinfo * ei, const char * filename)
{
    if (ei) ei->clear();

    // ����Դ�ļ�
    myflexer mf(fk, ei);
    
    FKLOG("fkparse %p %s", fk, filename);
    bool b = mf.inputfile(filename);
    if (!b)
    {
        FKERR("fkparse open %s fail", fk, filename);
        return 0;
    }

    // �����﷨����
    int ret = yyparse((void *)&mf); 
    if (ret != 0)
    {
        FKERR("fkparse yyparse %s fail ret %d", fk, filename, ret);
        fk->seterror(ei, efk_parse_file_fail, "parse %s file fail", filename);
        return 0;
    }
    
    FKLOG("fkparse yyparse %p %s OK", fk, filename);

    binary * bin = fknew<binary>(fk, fk);

    // ����
    compiler mc(fk, ei, bin);
    b = mc.compile(&mf);
    if (!b)
    {
        FKERR("fkparse  %s compile %s fail", fk, filename);
        return 0;
    }
    
    FKLOG("fkparse %p %s OK", fk, filename);
    
    return bin;
}

void delbinary(binary * bin)
{
    fuck *fk = bin->getfuck();
    fkdelete<binary>(fk, bin);
}

bool fkisfunc(binary * bin, const char * func)
{
    return bin->is_have_func(func);
}

// ���ú���
void fkrun(binary * bin, fkerrorinfo * ei, const char * func, paramstack * s)
{
    if (ei) ei->clear();

    fuck *fk = bin->getfuck();
    FKLOG("fkrun %p %p %s", fk, bin, func);
    
    routine r(fk, ei);
    r.entry(bin, func, s);
    
    processor p(fk);

    p.add(&r);

    int cmdnum = 0;
    int framenum = 0;
    int perframecmdnum = fk->m_per_frame_cmd_num;
    while (!p.isend())
    {
        cmdnum += p.run(perframecmdnum);
        framenum++;
    }
    
    FKLOG("fkrun cmdnum[%d] perframecmdnum[%d] framenum[%d]", cmdnum, perframecmdnum, framenum);

    s->push(r.getret());
    
    FKLOG("fkrun %p %p %s OK", fk, bin, func);
}

// ��������
paramstack * fknewparamstack(binary * bin)
{
    fuck *fk = bin->getfuck();
    return fknew<paramstack>(fk, fk);
}

void fkdeleteparamstack(paramstack * s)
{
    fuck *fk = s->getfuck();
    fkdelete<paramstack>(fk, s);
}

void fkpspushpointer(paramstack * s, void * p)
{
    fuck *fk = s->getfuck();
    variant v(p, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, char ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, unsigned char ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, short ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, unsigned short ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, int ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, unsigned int ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, float ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, double ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, std::string ret)
{
    fuck *fk = s->getfuck();
    variant v(ret.c_str(), fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, char * ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, const char * ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, bool ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, int64_t ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

template<>	void fkpspush(paramstack * s, uint64_t ret)
{
    fuck *fk = s->getfuck();
    variant v(ret, fk);
    s->push(v);
}

void fkpspoppointer(paramstack * s, void * & p)
{
    variant v;
    s->pop(v);
    p = (void*)v;
}

template<>	void fkpspop(paramstack * s)
{
    // nothing
}

template<>	char fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	unsigned char fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	short fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	unsigned short fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	int fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	unsigned int fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	float fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	double fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	std::string fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return ((String)v).c_str();
}

template<>	bool fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	int64_t fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}

template<>	uint64_t fkpspop(paramstack * s)
{
    variant v;
    s->pop(v);
    return v;
}


