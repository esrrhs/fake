#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

// �����
enum efkerror
{
    efk_ok,
    efk_open_file_fail,
    efk_open_file_empty,
    efk_parse_file_fail,
    efk_compile_same_func_name,
    efk_compile_func_arg_error,
    efk_compile_stmt_type_error,
	efk_compile_variable_not_found,
	efk_compile_explicit_type_error,
};

// �ű�����
struct fuck;

// ������
struct binary;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

// �������
fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0);
void delfuck(fuck * fk);

// ����
efkerror fkerror(fuck * fk);
const char * fkerrorstr(fuck * fk);

// �����ļ�
binary * fkparse(fuck * fk, const char * filename);

// ���ú���
bool fkrun(fuck * fk, binary * bin, const char * func);

