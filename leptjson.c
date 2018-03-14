#include<stdlib.h> 	/* NULL, malloc(), realloc(), free(), strtod() */
#include<assert.h>  /* assert() */
#include<errno.h>	/* errno, ERANGE */
#include<math.h>	/* HUGE_VAL */
#include"leptjson.h"

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef LEPT_PARSE_STRINGIFY_INIT_SIZE
#define LEPT_PARSE_STRINGIFY_INIT_SIZE 256
#endif

#define EXPECT(c, ch)   do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) 	((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c,ch)		do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)
#define PUTS(c, s, len) memcpy(lept_context_push(c, len), s, len)

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
/* 解析UTF-8编码 */
static const char* lept_parse_hex4(const char* p, unsigned u);
static void lept_encode_utf8(lept_context* c, unsigned u);
/* 解析数组 */
static int lept_parse_array(lept_context* c, lept_value* v);
/* 对堆栈的操作 */
static void* lept_context_push(lept_context* c, size_t size);
static void* lept_context_pop(lept_context* c, size_t size);
/* 生成JSON */
char* lept_stringify(const lept_value* v, size_t* length);
static void lept_stingify_value(lept_context* c, const lept_value* v);
static void lept_stringify_string(lept_context* c, const char* s, size_t len);

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
	v->u.num = strtod(c->json, NULL);
	//对于数字过大的处理
	if(errno == ERANGE && (v->u.num == HUGE_VAL ||v->u.num == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	c->json = p;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)
/* 解析字符串 */
static int lept_parse_string(lept_context* c, lept_value* v)
{
	size_t head = c->top, len;
	unsigned u, u2;
	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	for(;;) {
		char ch = *p++;
		switch(ch) {
			case'\"':
				len = c->top - head;
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
					case'u':  
							if(!(p = lept_parse_hex4(p, &u))) {
						 		STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
							}
							if(u >= 0xD800 && u <= 0xDBFF) {
									if(*p++ != '\\')
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
								if(*p++ != 'u')
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
								if(!(p = lept_parse_hex4(p, &u2)))
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
								if(u2 < 0xDC00 || u2 > 0xDFFF)
									STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
								u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
							}
							lept_encode_utf8(c,u);
							break;
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

/* 解析utf-8编码
 * 解析4位16进制数字(码点)*/
static const char* lept_parse_hex4(const char* p, unsigned* u) 
{
	int i;
	*u = 0;
	for(i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if		(ch >= '0' && ch <= '9')  *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
		else if NULL;
	}
	return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u)
{
	if(u <= 0x7F)
		PUTC(c, u & 0xFF);
	else if(u <= 0x7FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u		 & 0x3F));
	}
	else if(u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >>  6) & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >>  6) & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}
}

/* 解析数组 */
static int lept_parse_array(lept_context* c, lept_value* v)
{
	size_t i, size = 0;
	int ret;
	EXPECT(c, '[');
	lept_parse_whitespace(c);
	if(*c->json == ']') {
		c->json++;
		v->type = LEPT_ARRAY;
		v->u.arr.size = 0;
		v->u.arr.e = NULL;
		return LEPT_PARSE_OK;
	}
	for(;;) {
		lept_value e;
		lept_init(&e);
		if((ret = lrpt_parse_value(c, &e)) != LEPT_PARSE_OK) {
			return ret;
		}
		memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
		size++;
		lept_parse_whitespace(c);
		if(*c->json == ',') {
			c->json++;
			lept_parse_whitespace(c);
		} else if(*c->json == ']') {
			c->json++;
			v->type = LEPT_ARRAY;
			v->u.arr.size = size;
			size *= sizeof(lept_value);
			memcpy(v->u.arr.e = (lept_value*)malloc(size), lept_context_pop(c, size), size);
			return LEPT_PARSE_OK;
		} else {
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	for(i = 0; i < size; i++) {
		lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
	}
	return ret;
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

/*--------------------------------07BEG---------------------------*/
/* 调用自定义堆栈存储解析结果  */
//使用length而不是strlen()可以减少性能消耗
char* lept_stringify(const lept_value* v, size_t* length)
{
	lept_context c;
	assert(v != NULL);
	c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);
	c.top = 0;
	lept_stringify_value(&c, v);
	if(length) {
		*length = c.top;
	}
	PUTC(&c, '\0');
	return c.stack;
}

/* 对应类型转换成JSON值 */
static void lept_stringify_value(lept_context* c, const lept_value* v)
{
	size_t i;
	switch(v->type) {
		case LEPT_NULL:		PUTS(c, "null",  4); break;
		case LEPT_FALSE:	PUTS(c, "false", 5); break;
		case LEPT_NULL:		PUTS(c, "true",  4); break;
		case LEPT_NUMBER:	{
							char buffer[32];
							//"%.17g"用来把（最大）双精度浮点数转换成文本
							int length = sprintf(buffer, "%.17g", v->u.num);
							PUTS(c, buffer, length);
							}
							break;
		case LEPT_STRING:	lept_stringify_string(c, v->u.str.str, v->u.str.len); break;
		case LEPT_ARRAY:
			PUTC(c, '[');
			for(i = 0; i < v->u.a.size; i++) {
				if(i > 0)
					PUTC(c, ',');
				lept_stringify_value(c, &v->u.arr.e[i]);
			}
			PUTC(c, ']');
			break;
		case LEPT_OBJECT: 	//对象的东西最后再补充
		default: assert(0 && "invalid type");
	}
}

/* 把字符串类型转换成JSON值 */
static void lept_stringify_string(lept_context* c, const char* s, size_t len)
{
	static const char hex_digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	size_t i;
	char* head, *p;
	assert(s != NULL);
	//预先分配足够多的内存，避免多次压栈的开销
	//len * 6 + 2 : 每个字符最长\uOOXX六个字符加上两个双引号
	p = head = lept_context_push(c, size = len * 6 + 2);
	*p++ = '"';
	for(i = 0; i < len; i++) {
		unsigned char ch = (unsigned char)s[i];
		switch(ch) {
			//处理转义字符
			case'\"': 	*p++ = '\\'; *p++ = '\"'; break;
			case'\\': 	*p++ = '\\'; *p++ = '\\'; break;
			case'\b': 	*p++ = '\\'; *p++ = 'b';  break;
			case'\f': 	*p++ = '\\'; *p++ = 'f';  break;
			case'\n': 	*p++ = '\\'; *p++ = 'n';  break;
			case'\r': 	*p++ = '\\'; *p++ = 'r';  break;
			case'\t': 	*p++ = '\\'; *p++ = 't';  break;
			default:
				//这个if没搞明白
				if(ch < 0x20) {
					*p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
					*p++ = hex_digits[ch >> 4];
					*p++ = hex_digits[ch & 15];
				} else {
					*p++ = s[i];
				}
		}
	}
	*p++ = '"';
	c->top -= size - (p - head);
}

/*--------------------------------07END---------------------------*/

void lept_free(lept_value* v)
{
	size_t i;
	assert(v != NULL);
	switch(v->type) {
		case LEPT_STRING:
			free(v->u.str.str);
			break;
		case LEPT_ARRAY:
			for(i = 0; i < v->u.arr.size; i++) {
				lept_free(&v->u.arr.e[i]);
			}
			free(v->u.arr.e);
			break;
		default: break;
	}
	v->type = LEPT_NULL;
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
	assert(str != NULL || len == 0);
	lept_free(v);
	v->u.str.str = (char*)malloc(len + 1);
	memcpy(v->u.str.str, str, len);
	v->u.str.str[len] = '\0';
	v->u.str.len = len;
	v->type = LEPT_STRING;
}

size_t lept_get_array_size(const lept_value* v)
{
	assert(v != NULL);
	assert(v->type == LEPT_ARRAY);
	return v->u.arr.size;
}

lept_value* lept_get_array_element(const lept_value* v, size_t index)
{
	assert(v != NULL);
	assert(v->type == LEPT_ARRAY);
	assert(index < v->u.arr.size);
	return &v->u.arr.e[index];
}
