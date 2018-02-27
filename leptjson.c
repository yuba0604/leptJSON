#include<stdlib.h>
#include"leptjson.h"

typedef struct {
	const char* json;
}lept_context;

/*判断分词*/
static int lept_parse_value(lept_context* c, lept_value* v);
/*解析空格*/
static void lept_parse_whitespace(lept_context* c);
/*解析true*/
static int lept_parse_true(lept_context* c, lept_value* v);
/*解析false*/
static int lept_parse_false(lept_context* c, lept_value* v);
/*解析null*/
static int lept_parse_null(lept_context* c, lept_value* v);

/*解析函数*/
int lept_parse(lept_value* v, const char* json)
{
	lept_context c;
	int ret;
	c.json = json;
	v->type = NULL;
	//处理第一个空白字符
	lept_parse_whitespace(&c);
	//处理第二个实义字符
	if( (ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		//处理第三个空白字符
		lept_parse_whitespace(&c);
		if( *c.json != '\0' ) {
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

/*判断分词*/
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json) {
		case 't': return lept_parse_true(c, v);
		case 'f': return lept_parse_false(c, v);
		case 'n': return lept_parse_null(c, v);
		case '\0':return LEPT_PARSE_EXPECT_VALUE;
		default:  return LEPT_PARSE_INVALID_VALUE;
	}
}

/*解析空格*/
static void lept_parse_whitespace(lept_context* c, lept_value* v)
{
	const char *p = c->json;
	//读到空格，制表符，回车和换行符就跳过
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
		p++;
	}
	c->json = p;
}

/*解析true*/
static int lept_parse_true(lept_context* c, lept_value* v)
{	//没用宏，直接把判断首字母写进去了
	if(c->json[0] != 't' || c->json[1] != 'r' || c->json[2] != 'u' 
			|| c->json[3] != 'e' ) {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 4;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}

/*解析false*/
static int lept_parse_false(lept_context* c, lept_value* v)
{
	if(c->json[0] != 'f' || c->json[1] != 'a' || c->json[2] != 'l' 
			|| c->json[3] != 's' || c->json[4] = 'e') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 5;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}

/*解析null*/
static int lept_parse_null(lept_context* c, lept_value* v)
{
	if(c->json[0] != 'n' || c->json[1] != 'u' || c->json[2] != 'l' 
			|| c->json[3] != 'l' ) {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 4;
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}

//不知道这个函数干啥的
lept_type lept_get_type(const lept_value* v)
{
	return v->type;
}
