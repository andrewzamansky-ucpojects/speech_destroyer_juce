/*
 * file : project_func_declarations.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _project_func_declarations_H
#define _project_func_declarations_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void busy_delay(uint32_t mSec);

#ifdef CONFIG_ARMCC
 static size_t strnlen(const char *s, size_t n){return strlen(s);}
#endif

#endif /* */
