#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/*JSON的数据类型*/
typedef enum{ LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/*用来表示语法生成树的节点*/
typedef struct {
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
	LEPT_PARSE_ROOT_NOT_SINGULAR
};

/*读取函数*/
lept_type lept_get_type(const lept_value* v);

#endif /* LEPTJSON_H_*/
