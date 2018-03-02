#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include<stddef.h> /* size_t */
/*JSON的数据类型*/
typedef enum{ LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/*用来表示语法生成树的节点*/
typedef struct {
	union {
		struct {char* str; size_t len; }str;/*变长字符串*/
		double num; 						/*数字*/
	}u;
	lept_type type;
}lept_value;

/*解析JSON的函数
 *传入一个根节点指针，传入一个JSON文本字符串 */
int lept_parse(lept_value* v, const char* json);

/*lept_parse()的返回值类型*/
enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

void lept_free(lept_value* v);

lept_type lept_get_type(const lept_value* v);

#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

#endif /* LEPTJSON_H_*/
