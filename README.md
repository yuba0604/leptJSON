#leptJSON--简易JSON库
---
##简介
这是一个使用C语言编写的JSON库。特性如下：

 - 符合标准的 JSON 解析器和生成器
 - 手写的递归下降解析器
 - 仅支持 UTF-8 JSON 文本
 - 仅支持以 double 存储 JSON number 类型

---
##工作流程
解析过程：

 1. 解析字符
 2. 判断分词
 3. 进入相应分词的处理函数
 4. 生成词法分析串

生成过程：
 就是逆解析过程。

---
##所需要的JSON语法子集
`JSON-text = ws value ws
ws = *(%x20 / %x09 / %x0A / %x0D)
value = null / false / true / number / string / array / object `
JSON文本由三个部分组成，首先是空白（ws），接着是一个值，最后是空白(ws)。

空白(ws)，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成。

值(value)，是null(空值) 、 false(假) 、 true(真) 、 number(数字) 、 string(字符串) 、 array(数组) 、 object(对象)其中的一种。

`null  = "null"
false = "false"
true  = "true" `
null、false、true的值都是对应的字面量。

`number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit`
number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。只有整数是必需部分。

整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）。也就是说，0123 不是一个合法的 JSON 数字。

小数部分比较直观，就是小数点后是一或多个数字（0-9）。

JSON 可使用科学记数法，指数部分由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。

`string = quotation-mark *char quotation-mark
char = unescaped /
   escape (
       %x22 /          ; "    quotation mark  U+0022
       %x5C /          ; \    reverse solidus U+005C
       %x2F /          ; /    solidus         U+002F
       %x62 /          ; b    backspace       U+0008
       %x66 /          ; f    form feed       U+000C
       %x6E /          ; n    line feed       U+000A
       %x72 /          ; r    carriage return U+000D
       %x74 /          ; t    tab             U+0009
       %x75 4HEXDIG )  ; uXXXX                U+XXXX
escape = %x5C          ; \
quotation-mark = %x22  ; "
unescaped = %x20-21 / %x23-5B / %x5D-10FFFF`
JSON 字符串是由前后两个双引号(quotation-mark)夹着零至多个字符。

字符分为无转义字符或转义序列。转义序列有 9 种，都是以反斜线开始，如常见的 \n 代表换行符。

P.S.   '\u' Unicode编码先解析 4 位十六进制整数为码点，然后把这个码点编码成 UTF-8。


`array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D`
%x5B 是左中括号 [，%x2C 是逗号 , ，%x5D 是右中括号 ] ，ws 是空白字符。一个数组可以包含零至多个值，以逗号分隔，例如 []、[1,2,true]、[[1,2],[3,4],"abc"] 都是合法的数组。

`member = string ws %x3A ws value
object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D`
JSON 对象和 JSON 数组非常相似，区别包括 JSON 对象以花括号 {}（U+007B、U+007D）包裹表示，另外 JSON 对象由对象成员（member）组成，而 JSON 数组由 JSON 值组成。所谓对象成员，就是键值对，键必须为 JSON 字符串，然后值是任何 JSON 值，中间以冒号 :（U+003A）分隔。

---
##引用
本项目参考自https://github.com/miloyip/json-tutorial