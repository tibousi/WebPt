# WebPt һ����������վѹ�����Թ��ߣ�ʹ�ý��̲������̲߳���
a linux-side web pressure test tool,used by fork and openmp.Support different request method,proxy server and commands from text</br>
-h --help ��ӡ������Ϣ</br>
-c --clients ��Ϊ���õĲ�����ÿ�β��Ա����ã������������� -c 10����10������</br>
-t --time �ǳ����õĲ��������ò��Ե�ʱ�䣬Ĭ���Է���Ϊ��λ��������λҪ�Լ����ã�����-t 10�����Գ���10��</br>
-1 --http1.1 ʹ��http1.1</br>
-p --proxy ����server:port ,ʹ�ô��������</br>
--get ʹ��GET����</br>
--head ʹ��HEAD����</br>
--options ʹ��OPTIONS����</br>
--trace ʹ��TRACE����</br>
-P --post ʹ��POST����</br>
--cache ǿ�Ʋ�����</br>
-T --type ѡ��post�ύ��ʽ,֧��'application/x-www-form-urlencoded','multipart/form-data','application/json','text/xml'�����ύ��ʽ������1��4ѡ��</br>
-f --file ��ȡ�ı��ڵ����������г���(ֱ�������ı�·��,һ������һ�У���β������)</br>
-o --openmp ָ���߳���������-o 8,16���ϵ�ָ������Ч���˷������ڵ�ǰĿ¼�´���һ��tmp�ļ����Դ����ʱ�ļ�</br>
-u --url ָ�������ļ���url��ַ</br>
-s --save ָ�������ļ������֣����û�д˲�����Ĭ��Ϊԭ����������ǰĿ¼</br>