#include<stdlib.h> 	/* NULL, strtod() */
#include<assert.h>  /* assert() */
#include<errno.h>	/* errno, ERANGE */
#include<math.h>	/* HUGE_VAL */
#include"leptjson.h"

#define ISDIGIT(ch) 	((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct {
	const char* json;
}lept_context;

/* 判断分词 */
static int lept_parse_value(lept_context* c, lept_value* v);
/* 解析空格 */
static void lept_parse_whitespace(lept_context* c);
/* 解析简单字面量 */
static int lept_parse_literal(lept_context* c, lept_value* v,
		const char* literal, lept_type type);
/* 解析数字 */
static int lept_parse_number(lept_context* c, lept_value* v);

/* 解析函数 */
int lept_parse(lept_value* v, const char* json)
{
	lept_context c;
	int ret;
	assert(v != NULL);

	c.json = json;
	v->type = LEPT_NULL;
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

/* 判断分词 */
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json) {
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		case '\0':return LEPT_PARSE_EXPECT_VALUE;
		default:  return lept_parse_number(c, v);
	}
}

/* 解析空格 */
static void lept_parse_whitespace(lept_context* c)
{
	const char *p = c->json;
	//读到空格，制表符，回车和换行符就跳过
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
		p++;
	}
	c->json = p;
}

/* 解析简单字面量true, false, null */		
static int lept_parse_literal(lept_context* c, lept_value* v,
		const char* literal, lept_type type)
{
	size_t i;
	for(i = 0; literal[i]; i++) {
		if(c->json[i] != literal[i]) {
			return LEPT_PARSE_INVALID_VALUE;
		}
	}
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;
}

/* 解析数字 */
static int lept_parse_number(lept_context* c, lept_value* v)
{
	const char* p = c->json;
	//语法校验规则
	//负号部分
	if(*p == '-') p++;
	//整数部分
	if(*p == '0') p++;
	else {
		if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	//小数点
	if(*p == '.') {
		p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	//指数部分
	if(*p == 'e' || *p == 'E') {
		p++;
		if(*p == '+' || *p == '-') p++;
		if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
		for(p++; ISDIGIT(*p); p++);
	}
	v->num = strtod(c->json, NULL);
	//对于数字过大的处理
	if(errno == ERANGE && (v->num == HUGE_VAL ||v->num == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
}

lept_type lept_get_type(const lept_value* v)
{
	assert(v != NULL);

	return v->type;
}

double lept_get_number(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_NUMBER);

	return v->num;
}
