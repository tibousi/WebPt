#include "Header.h"
#include <signal.h>

//the part of surfing speed

void diect()
{
	//输出说明
	fprintf(stderr,
		"-h --help 打印帮助信息\n"
		"-c --clients 最为常用的参数，每次测试必设置，并发数量，例 -c10代表10个并发\n"
		"-t --time 非常常用的参数，设置测试的时间，默认以分钟为单位，其他单位要自己设置，例如-t 10，测试持续10秒\n"
		"-1 --http1.1 使用http1.1\n"
		//"-d --delay 指定时间延迟，在每个请求发出后，再随机延迟一段时间再发下一个\n"
		"-p --proxy 形似server:port ,使用代理服务器\n"
		"--get 使用GET请求\n"
		"--head 使用HEAD请求\n"
		"--options 使用OPTIONS请求\n"
		"--trace 使用TRACE请求\n"
		"-P --post 使用POST请求\n"
		"--cache 强制不缓存\n"
		"-T --type 选择post提交方式,支持'application/x-www-form-urlencoded','multipart/form-data','application/json','text/xml'四种提交方式，输入1到4选择\n"
		"-f --file 读取文本内的命令以运行程序(直接输入文本路径,一条命令一行，结尾请勿换行)\n"
		"-o --openmp 指定线程数，例如-o 8,16以上的指定将无效,此方法将在当前目录下创造一个tmp文件夹以存放临时文件\n"
		"-u --url 指定url，必设置的参数\n"
		"-s --save 指定存放路径文件名，如果没有此参数，默认为原名下载至当前目录\n"
		);
};


void deal_flag(int signal)
{
	//重置flag
	flag = 1;
}

void turn_url(const char *url)
{
	//转换url为统一格式
	strcpy(new_url, url);
	if (strlen(url) > 1024){
		fprintf(stderr, "URL is too long\n");
		exit(1);
	}
	if ((strncasecmp("http://", url, 7)) != 0)
	{
		char tmp_http[1024] = "http://";
		strcpy(new_url, strcat(tmp_http, url));
	}
	if (url[strlen(url) - 1] != '/')
	{
		strcpy(new_url, strcat(new_url, "/"));
	}
}

void build_request(const char *url)
{
	//根据不同参数建立连接
	bzero(host, 1024);
	bzero(request, 1024);

	if ((method == 2 && httpv != 1) || method == 3 && httpv != 1)
	{
		fprintf(stderr, "please use HTTP/1.1\n");
		exit(1);
	}

	strcpy(request, request_method[method]);

	turn_url(url);

	if (proxyhost != NULL)
	{
		strcat(request, new_url);
	}
	else
	{
		if (rindex(new_url, ':') != new_url + 4 && index(new_url + 5, ':') < index(new_url + 7, '/'))//http:// server:port形式
		{
			strncpy(host, new_url + 7, index(new_url + 5, ':') - new_url - 7);
			char tmp[6];
			strncpy(tmp, index(new_url + 5, ':') + 1, index(new_url + 7, '/') - index(new_url + 5, ':'));
			proxyport = atoi(tmp);//端口转换int
			if (proxyport == 0)
				proxyport = 80;//缺省值
		}
		else//如果没有指定端口号
		{
			strncpy(host, new_url + 7, index(new_url + 7, '/') - new_url - 7);//host为http://和结尾的/之间
		}
		strcat(request + strlen(request), index(new_url + 7, '/'));
	}
	if (httpv == 0)
		strcat(request, " HTTP/1.0\r\n");
	else
		strcat(request, " HTTP/1.1\r\n");

	strcat(request, "User - Agent: Mozilla / 5.0 (X11; Linux x86_64) AppleWebKit / 537(KHTML, like Gecko) Chrome / 47.0.2526Safari / 537.36\r\n");

	if (proxyhost == NULL)
		sprintf(request + strlen(request), "Host: %s\r\n", host);


	if (cache == 1)
	{
		if (httpv == 0)
			strcat(request, "Pragma: no-cache\r\n");
		else
			strcat(request, "Cache-Control: no-cache\r\n");
	}

	if (postflag == 1)
	{
		int pl = strlen(postdata);
		sprintf(request + strlen(request), "Content-Length: %d\r\n", pl);
		sprintf(request + strlen(request), "Content-Type: %s\r\n", post_type[posttype]);
		strcat(request, "\r\n");
		strcat(request, postdata);
		strcat(request, "\r\n");
	}

	if (httpv==1)
		strcat(request, "Connection: close\r\n");

	strcat(request, "\r\n");
}


void child(const char *host, const int port, const char *request)
{
	//fork之后的处理
	char recv[1024];
	struct sigaction sa;
	sa.sa_handler = deal_flag;
	sa.sa_flags = 0;
	int len = strlen(request);

	if (sigaction(SIGALRM, &sa, NULL))
		exit(1);
	alarm(testtime);

	Replace_goto(host, &port, request, &flag, &len, recv, &fail, &bytes, &success);

	if (flag == 1)
	{
		if (fail > 0) fail--;
	}
}

int deal_pid()
{
	//管道通信
	if (pipe(mypipe))
	{
		fprintf(stderr, "Build pipe failed");
		return 1;
	}

	pid_t pid;
	for (int i = 0; i < clients; i++)
	{
		pid = fork();
		if (pid < 0)
		{
			fprintf(stderr, "fork failed\n");
			return 1;
		}
		if (pid == 0)
			break;
	}


	FILE *f;
	if (pid == 0)
	{
		if (proxyhost == NULL)
			child(host, proxyport, request);
		else
			child(proxyhost, proxyport, request);

		f = fdopen(mypipe[1], "w");

		if (f == NULL)
		{
			fprintf(stderr, "write pipe error\n");
			return 1;
		}

		fprintf(f, "%d %d %d\n", success, fail, bytes);

		fclose(f);
		return 0;

	}
	else
	{
		f = fdopen(mypipe[0], "r");

		if (f == NULL)
		{
			fprintf(stderr, "open pipe error\n");
			return 1;
		}

		int s, fi, b;
		while (1)
		{
			int fr = fscanf(f, "%d %d %d", &s, &fi, &b);
			if (fr != 3)
			{
				fprintf(stderr, "pipe error\n");
				break;
			}

			success += s;
			fail += fi;
			bytes += b;
			clients--;

			if (clients == 0)
				break;
		}
		fclose(f);

		if (fileflag != 0)
		{
			sprintf(outcome + strlen(outcome), "The data of Example %d :\n", fileflag);
		}

		sprintf(outcome + strlen(outcome), "Speed of open the website:%s is %dpages\\min\n%d has succeed and %d has failed\nThe Average Netflow is %dKB\\min\nThis is your success rate:%.2f%\n",
			host,
			(int)((success + fail) / testtime * 60),
			success,
			fail,
			(int)(bytes / testtime/1024),
			success==0?0:(float)success / (success + fail)*100.00);

		return -1;

	}
	return 0;
}