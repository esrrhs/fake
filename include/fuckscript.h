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
    efk_openfilefail,
    efk_openfileempty,
};

struct fuck;

typedef void * (*fkmalloc)(size_t size);
typedef void (*fkfree)(void *ptr);

// �������
fuck * newfuck(fkmalloc fkm = 0, fkfree fkf = 0);
void delfuck(fuck * fk);

// ����
efkerror fkerror(fuck * fk);

// �����ļ�
bool fkparse(fuck * fk, const char * filename);

