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
#undef yyFlexLexer
#include <FlexLexer.h> 
#include "semantic.h"

#define MAX_FLEX_BUFFER_SIZE 1024*1024

class fuck;
class myflexer : public yyFlexLexer
{
public:
    // set parse command
    myflexer(fuck * fk) : m_fk(fk)
    {
        clear();
    }
    virtual ~myflexer() {}
    
    // flexͨ�������������ȡ��Ҫ�������ַ�����
    virtual int LexerInput(char* buf, int max_size);
    
    //������
    virtual void LexerError(const char* msg);

    // ��flex.cpp��ʵ��
    int yylex(YYSTYPE * yylval);

    // �����ļ�
    bool inputfile(const char * filename);

    void clear();
private:
    fuck * m_fk;
    char m_content[MAX_FLEX_BUFFER_SIZE];
    int m_num;
    int m_pos;
}; 

