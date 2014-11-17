#pragma once

#include <stdint.h>
#include <string>
#include <new>
#include <stdlib.h>

// �����
enum efkerror
{
    efk_ok = 0,
    efk_strsize = 100,
    
    efk_open_file_fail,
    efk_open_file_empty,
    efk_parse_file_fail,
    
    efk_compile_func_not_define,
    efk_compile_stack_identifier_error,
    efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,
	efk_compile_math_type_error,
	efk_compile_variable_has_define,
	efk_compile_add_stack_identifier,

	efk_reg_memfunc_double_name,
	
	efk_run_no_func_error,
};

// �ű�����
struct fake;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

#define FAKE_API extern "C"
#define MAX_FAKE_PARAM_NUM 10	// ���10������
#define MAX_FAKE_RETURN_NUM 10	// ���10������ֵ

struct fakeconfig
{
    fakeconfig() : fkm(&malloc), fkf(&free), 
        per_frame_cmd_num(100), 
        delete_routine_scale(4), 
        routine_grow_speed(100), 
        stack_list_grow_speed(100), 
        string_heap_num(100)
        {}
    fkmalloc fkm;
    fkfree fkf;	// �ڴ����
    int per_frame_cmd_num;			// ÿִ֡��������Ŀ
	int delete_routine_scale;		// ɾ����ЧЭ�̵ı���
	int routine_grow_speed;		// Э�������ٶȣ��ٷֱȣ�10%��������10%
	int stack_list_grow_speed;		// ջ���������ٶȣ��ٷֱȣ�10%��������10%
	int string_heap_num;    		// �ַ������ϵ������Ŀ
};

// �������
FAKE_API fake * newfake(fakeconfig * cfg = 0);
FAKE_API void delfake(fake * fk);

// �������
FAKE_API efkerror fkerror(fake * fk);
FAKE_API const char * fkerrorstr(fake * fk);

// �����ļ�
// ��֧���ڽű�������ֱ���滻�ű����µĽű������´�fkrun��ʱ���滻
FAKE_API bool fkparse(fake * fk, const char * filename);

// �Ƿ��к���
FAKE_API bool fkisfunc(fake * fk, const char * func);

// ��������
FAKE_API void fkpspushpointer(fake * fk, void * p);
FAKE_API void fkpspushchar(fake * fk, char ret);
FAKE_API void fkpspushuchar(fake * fk, unsigned char ret);
FAKE_API void fkpspushshort(fake * fk, short ret);
FAKE_API void fkpspushushort(fake * fk, unsigned short ret);
FAKE_API void fkpspushint(fake * fk, int ret);
FAKE_API void fkpspushuint(fake * fk, unsigned int ret);
FAKE_API void fkpspushfloat(fake * fk, float ret);
FAKE_API void fkpspushdouble(fake * fk, double ret);
FAKE_API void fkpspushcharptr(fake * fk, char * ret);
FAKE_API void fkpspushccharptr(fake * fk, const char * ret);
FAKE_API void fkpspushbool(fake * fk, bool ret);
FAKE_API void fkpspushint64(fake * fk, int64_t ret);
FAKE_API void fkpspushuint64(fake * fk, uint64_t ret);

template<typename T>  
inline void fkpspush(fake * fk, T ret)
{ 
    fkpspushpointer(fk, (void*)ret); 
}
template<>	inline void fkpspush(fake * fk, char ret)
{
	fkpspushchar(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned char ret)
{
	fkpspushuchar(fk, ret);
}

template<>	inline void fkpspush(fake * fk, short ret)
{
	fkpspushshort(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned short ret)
{
	fkpspushushort(fk, ret);
}

template<>	inline void fkpspush(fake * fk, int ret)
{
	fkpspushint(fk, ret);
}

template<>	inline void fkpspush(fake * fk, unsigned int ret)
{
	fkpspushuint(fk, ret);
}

template<>	inline void fkpspush(fake * fk, float ret)
{
	fkpspushfloat(fk, ret);
}

template<>	inline void fkpspush(fake * fk, double ret)
{
	fkpspushdouble(fk, ret);
}

template<>	inline void fkpspush(fake * fk, std::string ret)
{
	fkpspushccharptr(fk, ret.c_str());
}

template<>	inline void fkpspush(fake * fk, char * ret)
{
	fkpspushcharptr(fk, ret);
}

template<>	inline void fkpspush(fake * fk, const char * ret)
{
	fkpspushccharptr(fk, ret);
}

template<>	inline void fkpspush(fake * fk, bool ret)
{
	fkpspushbool(fk, ret);
}

template<>	inline void fkpspush(fake * fk, int64_t ret)
{
	fkpspushint64(fk, ret);
}

template<>	inline void fkpspush(fake * fk, uint64_t ret)
{
	fkpspushuint64(fk, ret);
}

FAKE_API void fkpspoppointer(fake * fk, void * & p);
FAKE_API char fkpspopchar(fake * fk);
FAKE_API unsigned char fkpspopuchar(fake * fk);
FAKE_API short fkpspopshort(fake * fk);
FAKE_API unsigned short fkpspopushort(fake * fk);
FAKE_API int fkpspopint(fake * fk);
FAKE_API unsigned int fkpspopuint(fake * fk);
FAKE_API float fkpspopfloat(fake * fk);
FAKE_API double fkpspopdouble(fake * fk);
FAKE_API const char * fkpspopcstrptr(fake * fk);
FAKE_API bool fkpspopbool(fake * fk);
FAKE_API int64_t fkpspopint64(fake * fk);
FAKE_API uint64_t fkpspopuint64(fake * fk);

template<typename T>  
inline T fkpspop(fake * fk)
{ 
    void * ret = 0; 
    fkpspoppointer(fk, ret); 
    return (T)ret; 
}

template<>	inline void fkpspop(fake * fk)
{
	// nothing
}

template<>	inline char fkpspop(fake * fk)
{
	return fkpspopchar(fk);
}

template<>	inline unsigned char fkpspop(fake * fk)
{
	return fkpspopuchar(fk);
}

template<>	inline short fkpspop(fake * fk)
{
	return fkpspopshort(fk);
}

template<>	inline unsigned short fkpspop(fake * fk)
{
	return fkpspopushort(fk);
}

template<>	inline int fkpspop(fake * fk)
{
	return fkpspopint(fk);
}

template<>	inline unsigned int fkpspop(fake * fk)
{
	return fkpspopuint(fk);
}

template<>	inline float fkpspop(fake * fk)
{
	return fkpspopfloat(fk);
}

template<>	inline double fkpspop(fake * fk)
{
	return fkpspopdouble(fk);
}

template<>	inline std::string fkpspop(fake * fk)
{
	const char * ret = fkpspopcstrptr(fk);
	return ret ? ret : "";
}

template<>	inline bool fkpspop(fake * fk)
{
	return fkpspopbool(fk);
}

template<>	inline int64_t fkpspop(fake * fk)
{
	return fkpspopint64(fk);
}

template<>	inline uint64_t fkpspop(fake * fk)
{
	return fkpspopuint64(fk);
}

FAKE_API void fkpsclear(fake * fk);

// �˺����ڲ�ʹ�ã��Ƽ�ʹ��ģ��
FAKE_API void fkrunps(fake * fk, const char * func);

// ���ú���������ִ��
template<typename RVal>
RVal fkrun(fake * fk, const char * func)
{
    fkpsclear(fk);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrun(fake * fk, const char * func, T1 arg1)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrun(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkpspush<T5>(fk, arg5);
    fkrunps(fk, func);
    return fkpspop<RVal>(fk);
}


// �˺����ڲ�ʹ�ã��Ƽ�ʹ��ģ��
FAKE_API void fkrunpsjit(fake * fk, const char * func);

// ���ú���,native code
template<typename RVal>
RVal fkrunjit(fake * fk, const char * func)
{
    fkpsclear(fk);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1>
RVal fkrunjit(fake * fk, const char * func, T1 arg1)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
RVal fkrunjit(fake * fk, const char * func, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
    fkpsclear(fk);
    fkpspush<T1>(fk, arg1);
    fkpspush<T2>(fk, arg2);
    fkpspush<T3>(fk, arg3);
    fkpspush<T4>(fk, arg4);
    fkpspush<T5>(fk, arg5);
    fkrunpsjit(fk, func);
    return fkpspop<RVal>(fk);
}

// һ����Ա����ָ���ŵĿռ䣬һ��16���ֽ�
#define FAKE_MEMFUNC_SIZE 32

// functor
struct fkfunctor;
typedef void (*fkcfunction) (fake * fk, fkfunctor * ff);
struct fkfunctor
{
    fkfunctor() : ff(0), param1(0) {}
    fkfunctor(fkcfunction _ff, void * _param1) : ff(_ff), param1(_param1) {}
    template <typename F>
    fkfunctor(fkcfunction _ff, void * _param1, F f) : ff(_ff), param1(_param1) 
    {
        new(param2) F(f);
    }
    fkcfunction ff;
    void * param1;
    char param2[FAKE_MEMFUNC_SIZE];
};

template<typename RVal, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
struct fkinvoker
{
	static void invoke(fake * fk, fkfunctor * ff) 
	{ 
	    RVal ret = ((RVal(*)(T1,T2,T3,T4,T5))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk),
	        fkpspop<T5>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
struct fkinvoker<RVal, T1, T2, T3, T4> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    RVal ret = ((RVal(*)(T1,T2,T3,T4))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2, typename T3>
struct fkinvoker<RVal, T1, T2, T3> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    RVal ret = ((RVal(*)(T1,T2,T3))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1, typename T2>
struct fkinvoker<RVal, T1, T2> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    RVal ret = ((RVal(*)(T1,T2))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T1>
struct fkinvoker<RVal, T1> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    RVal ret = ((RVal(*)(T1))ff->param1)(fkpspop<T1>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal>
struct fkinvoker<RVal>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    RVal ret = ((RVal(*)(void))ff->param1)();
	    fkpspush<RVal>(fk, ret);
	}
};

//
template<typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkinvoker<void, T1, T2, T3, T4, T5>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    ((void(*)(T1,T2,T3,T4,T5))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk),
	        fkpspop<T5>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2, typename T3, typename T4>
struct fkinvoker<void, T1, T2, T3, T4>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    ((void(*)(T1,T2,T3,T4))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2, typename T3>
struct fkinvoker<void, T1, T2, T3>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    ((void(*)(T1,T2,T3))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T1, typename T2>
struct fkinvoker<void, T1, T2>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    ((void(*)(T1,T2))ff->param1)(fkpspop<T1>(fk),
	        fkpspop<T2>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T1>
struct fkinvoker<void, T1>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    ((void(*)(T1))ff->param1)(fkpspop<T1>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<>
struct fkinvoker<void>
{
	static void invoke(fake * fk, fkfunctor * ff) 
	{
	    ((void(*)(void))ff->param1)();
	    fkpspush<int>(fk, 0);
	}
};

FAKE_API void fkpushfunctor(fake * fk, const char * name, fkfunctor ff);

// ע��C����
template<typename RVal>
void fkreg(fake * fk, const char * name, RVal (*func)())
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal>::invoke, (void*)func));
}

template<typename RVal, typename T1>
void fkreg(fake * fk, const char * name, RVal (*func)(T1))
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2))
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3))
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4))
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4>::invoke, (void*)func));
}

template<typename RVal, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (*func)(T1, T2, T3, T4, T5))
{
    fkpushfunctor(fk, name, fkfunctor(fkinvoker<RVal, T1, T2, T3, T4, T5>::invoke, (void*)func));
}

template<typename RVal, typename T, typename T1=void, typename T2=void, typename T3=void, typename T4=void, typename T5=void>
struct fkmeminvoker
{
	static void invoke(fake * fk, fkfunctor * ff) 
	{ 
	    T * p = fkpspop<T*>(fk);    // ��ͬ������˳��һ�����������ȫ
	    RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4,T5))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk),
	        fkpspop<T5>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
struct fkmeminvoker<RVal, T, T1, T2, T3, T4> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3,T4))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2, typename T3>
struct fkmeminvoker<RVal, T, T1, T2, T3> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    RVal ret = ((p)->*(*(RVal(T::**)(T1,T2,T3))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1, typename T2>
struct fkmeminvoker<RVal, T, T1 ,T2> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    RVal ret = ((p)->*(*(RVal(T::**)(T1,T2))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T, typename T1>
struct fkmeminvoker<RVal, T, T1> 
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    RVal(T::*func)(T1) = *(RVal(T::**)(T1))(ff->param2);
	    RVal ret = ((p)->*(func))(fkpspop<T1>(fk));
	    fkpspush<RVal>(fk, ret);
	}
};

template<typename RVal, typename T>
struct fkmeminvoker<RVal, T>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    RVal ret = ((p)->*(*(RVal(T::**)(void))(ff->param2)))();
	    fkpspush<RVal>(fk, ret);
	}
};

//
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
struct fkmeminvoker<void, T, T1, T2, T3, T4, T5>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(T1,T2,T3,T4,T5))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk),
	        fkpspop<T5>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2, typename T3, typename T4>
struct fkmeminvoker<void, T, T1, T2, T3, T4>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(T1,T2,T3,T4))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk),
	        fkpspop<T4>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2, typename T3>
struct fkmeminvoker<void, T, T1, T2, T3>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(T1,T2,T3))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk),
	        fkpspop<T3>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1, typename T2>
struct fkmeminvoker<void, T, T1, T2>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(T1,T2))(ff->param2)))(fkpspop<T1>(fk),
	        fkpspop<T2>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T, typename T1>
struct fkmeminvoker<void, T, T1>
{
	static void invoke(fake * fk, fkfunctor * ff)
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(T1))(ff->param2)))(fkpspop<T1>(fk));
	    fkpspush<int>(fk, 0);
	}
};

template<typename T>
struct fkmeminvoker<void, T>
{
	static void invoke(fake * fk, fkfunctor * ff) 
	{
	    T * p = fkpspop<T*>(fk);
	    ((p)->*(*(void(T::**)(void))(ff->param2)))();
	    fkpspush<int>(fk, 0);
	}
};

// ע���ຯ������֧���ຯ������
template<typename RVal, typename T>
void fkreg(fake * fk, const char * name, RVal (T::*func)())
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1))
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2))
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3))
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4))
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4>::invoke, 0, func));
}

template<typename RVal, typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
void fkreg(fake * fk, const char * name, RVal (T::*func)(T1, T2, T3, T4, T5))
{
    fkpushfunctor(fk, name, fkfunctor(fkmeminvoker<RVal, T, T1, T2, T3, T4, T5>::invoke, 0, func));
}

// �����������ú���
FAKE_API void fkopenbaselib(fake * fk);

// profile 
FAKE_API void fkopenprofile(fake * fk);
FAKE_API void fkcloseprofile(fake * fk);
FAKE_API const char * fkdumpprofile(fake * fk);
