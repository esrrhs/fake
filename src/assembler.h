#pragma once

#include "types.h"
#include "codegen.h"
#include "syntree.h"
#include "asmgen.h"

// ÿ��bytecode pos -> machinecode pos��ȫ�����ݣ������滻��תָ��Ļ���ָ���ַ
typedef std::map<int, int> posmap;
// ����Ƚ���jump����û��jumpĿ���ʵ�ʵ�ַ�����ȴ���������������һ���滻
// keyΪjump��ַ�ڻ������е�λ�ã�valueΪbytecode pos
typedef std::map<int, int> caremap;

struct fake;
class native;
class binary;
class func_binary;
class assembler
{
public:
    assembler(fake * fk, native * nt) : m_fk(fk), m_native(nt)
    {
    }
    ~assembler() 
    {
        clear();
    }

    void clear();
    bool compile(binary * bin);

private:
    bool compile_func(const func_binary & fb);
    bool compile_next(asmgen & asg, const func_binary & fb);
    bool compile_assign(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_return(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_math(asmgen & asg, const func_binary & fb, command cmd);
	bool compile_math_assign(asmgen & asg, const func_binary & fb, command cmd);
	bool compile_cmp(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_jne(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_jmp(asmgen & asg, const func_binary & fb, command cmd);
    bool compile_call(asmgen & asg, const func_binary & fb, command cmd);
    
private:
    fake * m_fk;
    native * m_native;
	int m_pos;
	posmap m_posmap;
	caremap m_caremap;
};

