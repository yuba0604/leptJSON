#leptJSON--����JSON��
---
##���
����һ��ʹ��C���Ա�д��JSON�⡣�������£�

 - ���ϱ�׼�� JSON ��������������
 - ��д�ĵݹ��½�������
 - ��֧�� UTF-8 JSON �ı�
 - ��֧���� double �洢 JSON number ����

---
##��������
�������̣�

 1. �����ַ�
 2. �жϷִ�
 3. ������Ӧ�ִʵĴ�����
 4. ���ɴʷ�������

���ɹ��̣�
 ������������̡�

---
##����Ҫ��JSON�﷨�Ӽ�
`JSON-text = ws value ws
ws = *(%x20 / %x09 / %x0A / %x0D)
value = null / false / true / number / string / array / object `
JSON�ı�������������ɣ������ǿհף�ws����������һ��ֵ������ǿհ�(ws)��

�հ�(ws)������������ո����space U+0020�����Ʊ����tab U+0009�������з���LF U+000A�����س�����CR U+000D������ɡ�

ֵ(value)����null(��ֵ) �� false(��) �� true(��) �� number(����) �� string(�ַ���) �� array(����) �� object(����)���е�һ�֡�

`null  = "null"
false = "false"
true  = "true" `
null��false��true��ֵ���Ƕ�Ӧ����������

`number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digit`
number ����ʮ���Ʊ�ʾ������Ҫ�� 4 ����˳����ɣ����š�������С����ָ����ֻ�������Ǳ��貿�֡�

������������� 0 ��ʼ��ֻ���ǵ��� 0������ 1-9 ��ʼ�Ļ������Լ��������������֣�0-9����Ҳ����˵��0123 ����һ���Ϸ��� JSON ���֡�

С�����ֱȽ�ֱ�ۣ�����С�������һ�������֣�0-9����

JSON ��ʹ�ÿ�ѧ��������ָ�������ɴ�д E ��Сд e ��ʼ��Ȼ����������ţ�֮����һ�������֣�0-9����

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
JSON �ַ�������ǰ������˫����(quotation-mark)������������ַ���

�ַ���Ϊ��ת���ַ���ת�����С�ת�������� 9 �֣������Է�б�߿�ʼ���糣���� \n �����з���

P.S.   '\u' Unicode�����Ƚ��� 4 λʮ����������Ϊ��㣬Ȼ������������� UTF-8��


`array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D`
%x5B ���������� [��%x2C �Ƕ��� , ��%x5D ���������� ] ��ws �ǿհ��ַ���һ��������԰����������ֵ���Զ��ŷָ������� []��[1,2,true]��[[1,2],[3,4],"abc"] ���ǺϷ������顣

`member = string ws %x3A ws value
object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D`
JSON ����� JSON ����ǳ����ƣ�������� JSON �����Ի����� {}��U+007B��U+007D��������ʾ������ JSON �����ɶ����Ա��member����ɣ��� JSON ������ JSON ֵ��ɡ���ν�����Ա�����Ǽ�ֵ�ԣ�������Ϊ JSON �ַ�����Ȼ��ֵ���κ� JSON ֵ���м���ð�� :��U+003A���ָ���

---
##����
����Ŀ�ο���https://github.com/miloyip/json-tutorial