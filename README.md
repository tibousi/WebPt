# WebPt 一个基础的网站压力测试工具，灵感来源：Siege，使用进程并发
a linux-side web pressure test tool,used by fork.Support different request method,proxy server and commands from text</br>
-h --help 打印帮助信息</br>
-c --clients 最为常用的参数，每次测试必设置，并发数量，例 -c 10代表10个并发</br>
-t --time 非常常用的参数，设置测试的时间，默认以分钟为单位，其他单位要自己设置，例如-t 10，测试持续10秒</br>
-1 --http1.1 使用http1.1</br>
-p --proxy 形似server:port ,使用代理服务器</br>
--get 使用GET请求</br>
--head 使用HEAD请求</br>
--options 使用OPTIONS请求</br>
--trace 使用TRACE请求</br>
-P --post 使用POST请求</br>
--cache 强制不缓存</br>
-T --type 选择post提交方式,支持'application/x-www-form-urlencoded','multipart/form-data','application/json','text/xml'四种提交方式，输入1到4选择</br>
-f --file 读取文本内的命令以运行程序(直接输入文本路径,一条命令一行，结尾请勿换行)</br>
