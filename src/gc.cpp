#include "gc.h"
#include "fake.h"

gc::gc(fake * fk) : m_fk(fk)
{
    ARRAY_INI(m_ret, fk);
    ARRAY_INI(m_entry, fk);
}

gc::~gc()
{
}

void gc::clear()
{
    ARRAY_DELETE(m_ret);
    ARRAY_DELETE(m_entry);
}

void gc::check(bool forcegc)
{
    m_fk->con.checkgc(forcegc);
    m_fk->sh.checkgc(forcegc);
    m_fk->ph.checkgc(forcegc);
}

const char * gc::dump()
{
	m_dumpstr.clear();
	return m_dumpstr.c_str();
}

#define PUSH_RET(ret, ss) \
    \
    if (UNLIKE(ARRAY_SIZE(ret) >= ARRAY_MAX_SIZE(ret))) \
    { \
        size_t newsize = ARRAY_SIZE(ret) + 1 + ARRAY_MAX_SIZE(ret) * (m_fk->cfg.array_grow_speed) / 100; \
        ARRAY_GROW(ret, newsize, void *); \
    } \
    ARRAY_PUSH_BACK(ret); \
    ARRAY_BACK(ret) = (void *)ss; \
    \
    FKLOG("PUSH_RET %p add string %s", m_fk, v->data.str->s);


#define PUSH_ENTRY(ret, v, find) \
    if (UNLIKE(!find.get((void*)v))) \
    { \
        if (UNLIKE(ARRAY_SIZE(entry) >= ARRAY_MAX_SIZE(entry))) \
        { \
            size_t newsize = ARRAY_SIZE(entry) + 1 + ARRAY_MAX_SIZE(entry) * (m_fk->cfg.array_grow_speed) / 100; \
            ARRAY_GROW(entry, newsize, variant *); \
        } \
        ARRAY_PUSH_BACK(entry); \
        ARRAY_BACK(entry) = (variant*)(v); \
        find.add((void*)(v)); \
        FKLOG("PUSH_ENTRY %p add %s %s", m_fk, vartypetostring((v)->type), vartostring(v).c_str()); \
    } \
    else \
    { \
        FKLOG("PUSH_ENTRY %p looped %s %s", m_fk, vartypetostring((v)->type), vartostring(v).c_str()); \
    }

array<void *> & gc::get_used_stringele()
{
    array<void *> & ret = get_used_by_type(variant::STRING);
    return ret;
}

array<void *> & gc::get_used_pointer()
{
    array<void *> & ret = get_used_by_type(variant::POINTER);
    return ret;
}

array<void *> & gc::get_used_by_type(int type)
{
    FKLOG("start get_used_by_type %p", m_fk);

    array<void *> & ret = m_ret;
    ARRAY_CLEAR(ret);

    array<processor *> & curprocessor = m_fk->rn.curprocessor;

    array<variant *> & entry = m_entry;
    ARRAY_CLEAR(entry);

    fkhashset<void *> find(m_fk);

    for (int i = 0; LIKE(i < (int)ARRAY_SIZE(curprocessor)); i++)
    {
        processor * pro = ARRAY_GET(curprocessor, i);

        for (int j = 0; LIKE(j < (int)ARRAY_SIZE(pro->m_pl.l)); j++)
        {
            routine * n = ARRAY_GET(pro->m_pl.l, j);

            int bp = n->m_interpreter.m_bp;
            const func_binary * fb = n->m_interpreter.m_fb;

            while (LIKE((!BP_END(bp))))
            {
                for (int z = 0; LIKE(z < fb->m_maxstack); z++)
                {
                    variant * v = 0;
                    GET_STACK_BY_INTER(bp, v, z, n->m_interpreter);
                    if (UNLIKE(v->type == type))
                    {
                        PUSH_RET(ret, v->data.p);
                    }
                    else if (UNLIKE(v->type == variant::ARRAY ||
                                    v->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, v, find);
                    }
                }

                BP_GET_FB_BY_INTER(bp, fb, n->m_interpreter);
                int callbp = 0;
                BP_GET_BP_BY_INTER(bp, callbp, n->m_interpreter);
                bp = callbp;
                if (UNLIKE(BP_END(bp)))
                {
                    break;
                }
            }

        }
    }

    variant_map * m = m_fk->con.newgmap();
    variant v;
    V_SET_MAP(&v, m);
    PUSH_ENTRY(ret, &v, find);

    while (UNLIKE(!ARRAY_EMPTY(entry)))
    {
        variant * v = ARRAY_BACK(entry);
        ARRAY_POP_BACK(entry);
        if (LIKE(v->type == type))
        {
            PUSH_RET(ret, v->data.p);
        }
        else if (UNLIKE(v->type == variant::ARRAY))
        {
            variant_array * va = v->data.va;

            if (UNLIKE(va->isconst))
            {
                continue;
            }

            for (int i = 0; LIKE(i < (int)va->va.m_max_size); ++i)
            {
                variant * n = ARRAY_GET(va->va, i);
                if (LIKE(n != 0))
                {
                    if (UNLIKE(n->type == type))
                    {
                        PUSH_RET(ret, n->data.str);
                    }
                    else if (UNLIKE(n->type == variant::ARRAY ||
                                    n->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, n, find);
                    }
                }
            }
        }
        else if (UNLIKE(v->type == variant::MAP))
        {
            variant_map * vm = v->data.vm;

            if (UNLIKE(vm->isconst))
            {
                continue;
            }

            for (const fkhashmap<variant, variant *>::ele * p = vm->vm.first(); p != 0; p = vm->vm.next())
            {
                const variant *key = &(p->k);
                const variant *value = *p->t;

                if (UNLIKE(key->type == type))
                {
                    PUSH_RET(ret, key->data.p);
                }
                else if (UNLIKE(key->type == variant::ARRAY ||
                                key->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, key, find);
                }

                if (UNLIKE(value->type == type))
                {
                    PUSH_RET(ret, value->data.p);
                }
                else if (UNLIKE(value->type == variant::ARRAY ||
                                value->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, value, find);
                }
            }
        }
    }

    ARRAY_CLEAR(entry);

    FKLOG("end get_used_by_type %p %u", m_fk, ARRAY_SIZE(ret));

    return ret;
}

array<void *> & gc::get_used_container()
{
    FKLOG("start get_used_container %p", m_fk);

    array<void *> & ret = m_ret;
    ARRAY_CLEAR(ret);

    array<processor *> & curprocessor = m_fk->rn.curprocessor;

    array<variant *> & entry = m_entry;
    ARRAY_CLEAR(entry);

    fkhashset<void *> find(m_fk);

    for (int i = 0; LIKE(i < (int)ARRAY_SIZE(curprocessor)); i++)
    {
        processor * pro = ARRAY_GET(curprocessor, i);

        for (int j = 0; LIKE(j < (int)ARRAY_SIZE(pro->m_pl.l)); j++)
        {
            routine * n = ARRAY_GET(pro->m_pl.l, j);
            if (UNLIKE(!n))
            {
                continue;
            }

            int bp = n->m_interpreter.m_bp;
            const func_binary * fb = n->m_interpreter.m_fb;

            while (LIKE((!BP_END(bp))))
            {
                for (int z = 0; LIKE(z < fb->m_maxstack); z++)
                {
                    variant * v = 0;
                    GET_STACK_BY_INTER(bp, v, z, n->m_interpreter);
                    if (UNLIKE(v->type == variant::ARRAY ||
                                    v->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, v, find);
                    }
                }

                BP_GET_FB_BY_INTER(bp, fb, n->m_interpreter);
                int callbp = 0;
                BP_GET_BP_BY_INTER(bp, callbp, n->m_interpreter);
                bp = callbp;
                if (UNLIKE(BP_END(bp)))
                {
                    break;
                }
            }

        }
    }

    variant_map * m = m_fk->con.newgmap();
    variant v;
    V_SET_MAP(&v, m);
    PUSH_ENTRY(ret, &v, find);

    while (UNLIKE(!ARRAY_EMPTY(entry)))
    {
        variant * v = ARRAY_BACK(entry);
        ARRAY_POP_BACK(entry);

        if (UNLIKE(v->type == variant::ARRAY))
        {
            variant_array * va = v->data.va;

            if (UNLIKE(va->isconst))
            {
                continue;
            }

            PUSH_RET(ret, va);

            for (int i = 0; LIKE(i < (int)va->va.m_max_size); ++i)
            {
                variant * n = ARRAY_GET(va->va, i);
                if (LIKE(n != 0))
                {
                    if (UNLIKE(n->type == variant::ARRAY ||
                                    n->type == variant::MAP))
                    {
                        PUSH_ENTRY(ret, n, find);
                    }
                    else
                    {
                        PUSH_RET(ret, n);
                    }
                }
            }
        }
        else if (UNLIKE(v->type == variant::MAP))
        {
            variant_map * vm = v->data.vm;

            if (UNLIKE(vm->isconst))
            {
                continue;
            }

            PUSH_RET(ret, vm);

            for (const fkhashmap<variant, variant *>::ele * p = vm->vm.first(); p != 0; p = vm->vm.next())
            {
                const variant *key = &(p->k);
                const variant *value = *p->t;

                if (UNLIKE(key->type == variant::ARRAY ||
                                key->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, key, find);
                }
                else
                {
                    PUSH_RET(ret, key);
                }

                if (UNLIKE(value->type == variant::ARRAY ||
                                value->type == variant::MAP))
                {
                    PUSH_ENTRY(ret, value, find);
                }
                else
                {

                    PUSH_RET(ret, value);
                }
            }
        }
        else
        {
            PUSH_RET(ret, v);
        }
    }

    ARRAY_CLEAR(entry);

    FKLOG("end get_used_container %p %u", m_fk, ARRAY_SIZE(ret));

    return ret;
}

