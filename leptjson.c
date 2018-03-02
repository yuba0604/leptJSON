#include<stdlib.h> 	/* NULL, malloc(), realloc(), free(), strtod() */
#include<assert.h>  /* assert() */
#include<errno.h>	/* errno, ERANGE */
#include<math.h>	/* HUGE_VAL */
#include"leptjson.h"

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)   do { assert(*c->json == (ch)); c->json++; } whlie(0)
#define ISDIGIT(ch) 	((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c,ch)		do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)

typedef struct {
	const char* json;
	//这是实现变长字符数组的堆栈
	char* stack;
	size_t size, top;
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
/* 解析字符串 */
static int lept_parse_string(lept_context* c, lept_value* v);
/* 对堆栈的操作 */
static void* lept_context_push(lept_context* c, size_t size);
static void* lept_context_pop(lept_context* c, size_t size);

/* 解析函数 */
int lept_parse(lept_value* v, const char* json)
{
	lept_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	lept_init(v);
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
	//确保释放前所有数据被弹出
	assert(c.top == 0);
	free(c.stack);
	return ret;
}

/* 判断分词 */
static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch(*c->json) {
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		default:  return lept_parse_number(c, v);
		case '"': return lept_parse_string(c, v);
		case '\0':return LEPT_PARSE_EXPECT_VALUE;
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

/* 解析字符串 */
static int lept_parse_string(lept_context* c, lept_value* v)
{
	size_t head = c->top, len;
	const char* p;
	EXPECT(c, '\"');
	p->json;
	for(;;) {
		switch(ch) {
			case'\"':
				len = c->top = head;
				lept_set_string(v, (const char*)lept_context_pop(c, len), len);
				c->json = p;
				return LEPT_PARSE_OK;
			case'\\':
				switch(*p++) {
					case'\"': PUTC(c, '\"'); break;
					case'\\': PUTC(c, '\\'); break;
					case'/':  PUTC(c, '/');  break;
					case'b':  PUTC(c, '\b'); break;
					case'f':  PUTC(c, '\f'); break;
					case'n':  PUTC(c, '\n'); break;
					case'r':  PUTC(c, '\r'); break;
					case't':  PUTC(c, '\t'); break;
					default:
						c->top = head;
						return LEPT_PARSE_INVALID_STRING_ESCAPE;
				}
			case'\0':
				c->top = head;
				return LEPT_PARSE_MISS_QUOTATION_MARK;
			default:
				if((unsigned char)ch < 0x20) {
					c->top = head;
					return LEPT_PARSE_INVALID_STRING_CHAR;
				}
				PUTC(c, ch);
		}
	}
}

/* 对堆栈的操作 */
static void* lept_context_push(lept_context* c, size_t size)
{
	void* ret;
	assert(size > 0);
	if(c->top + size >= c->size) {
		if(c->size == 0) {
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		}
		while(c->top + size >= c->size) {
			c->size += c->size >> 1; 	//将堆栈以1.5倍扩大
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void* lept_context_pop(lept_context* c, size_t size)
{
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}

/* 访问成员函数 */
lept_type lept_get_type(const lept_value* v)
{
	assert(v != NULL);
	return v->type;
}

int lept_get_boolean(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_TRUE || v->type == LEPT_FALSE);
	return v->type == LEPT_TRUE;
}

void lept_set_boolean(lept_value* v, int b)
{
	lept_free(v);
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_NUMBER);
	return v->u.num;
}

void lept_set_number(lept_value* v, double num)
{
	lept_free(v);
	v->u.num = num;
	v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_STRING);
	return v->u.str.str;
}

size_t lept_get_string_length(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_STRING);
	return v->u.str.len;
}

void lept_set_string(lept_value* v, const char* str, size_t len)
{
	assert(v != NULL);
	assert(s != NULL || len == 0);
	lept_free(v);
	v->u.str.str = (char*)malloc(len + 1);
	memcpy(v->u.str.str, str, len);
	v->u.str.str[len] = '\0';
	v->u.str.len = len;
	v->type = LEPT_STRING;
}
