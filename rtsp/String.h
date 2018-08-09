#ifndef _STRING_H_
#define _STRING_H_

#include<stdio.h>
#include<stdarg.h>
#include <string>
#include <string.h>
#include <stdlib.h>
using namespace std;

//typedef   string  	String;

class String :public string
{
	public:
		String(){};
		String(char*content){setTo(content);};
		const char *string(){return string::data();}
		void setTo(const char *to){clear();string::append(to);}
		int appendFormat(const char *pcFormat, ...)
		{
			char strTmpBuf[1000];
			va_list tArg;
			int iNum;

			/* 可变参数的处理, 抄自glibc的printf函数 */
			va_start (tArg, pcFormat);
			iNum = vsprintf (strTmpBuf, pcFormat, tArg);
			va_end (tArg);
			strTmpBuf[iNum] = '\0';
			string::append(strTmpBuf);

			return 0;
		}

		int append(const char *content)
		{
			string::append(content);
			return 0;
		}

		int append(const char *content,int size)
		{
			char *tmp = (char*)malloc(size *sizeof(char));
			if(tmp == NULL)
			{
				return -1;
			}

			memcpy(tmp,content,size);
			append(tmp);
			free(tmp);
			tmp = NULL;
			return 0;
		}

		int isEmpty()
		{
			return empty();
		}

		void setTo(const char *to,int size){
			setTo(to);
		}
};

#endif//_STRING_H_