#include "Header.h"
#include <signal.h>

//the part of surfing speed

void diect()
{
	//���˵��
	fprintf(stderr,
		"-h --help ��ӡ������Ϣ\n"
		"-c --clients ��Ϊ���õĲ�����ÿ�β��Ա����ã������������� -c10����10������\n"
		"-t --time �ǳ����õĲ��������ò��Ե�ʱ�䣬Ĭ���Է���Ϊ��λ��������λҪ�Լ����ã�����-t 10�����Գ���10��\n"
		"-1 --http1.1 ʹ��http1.1\n"
		//"-d --delay ָ��ʱ���ӳ٣���ÿ�����󷢳���������ӳ�һ��ʱ���ٷ���һ��\n"
		"-p --proxy ����server:port ,ʹ�ô��������\n"
		"--get ʹ��GET����\n"
		"--head ʹ��HEAD����\n"
		"--options ʹ��OPTIONS����\n"
		"--trace ʹ��TRACE����\n"
		"-P --post ʹ��POST����\n"
		"--cache ǿ�Ʋ�����\n"
		"-T --type ѡ��post�ύ��ʽ,֧��'application/x-www-form-urlencoded','multipart/form-data','application/json','text/xml'�����ύ��ʽ������1��4ѡ��\n"
		"-f --file ��ȡ�ı��ڵ����������г���(ֱ�������ı�·��,һ������һ�У���β������)\n"
		"-o --openmp ָ���߳���������-o 8,16���ϵ�ָ������Ч,�˷������ڵ�ǰĿ¼�´���һ��tmp�ļ����Դ����ʱ�ļ�\n"
		"-u --url ָ��url�������õĲ���\n"
		"-s --save ָ�����·���ļ��������û�д˲�����Ĭ��Ϊԭ����������ǰĿ¼\n"
		);
};


void deal_flag(int signal)
{
	//����flag
	flag = 1;
}

void turn_url(const char *url)
{
	//ת��urlΪͳһ��ʽ
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
	//���ݲ�ͬ������������
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
		if (rindex(new_url, ':') != new_url + 4 && index(new_url + 5, ':') < index(new_url + 7, '/'))//http:// server:port��ʽ
		{
			strncpy(host, new_url + 7, index(new_url + 5, ':') - new_url - 7);
			char tmp[6];
			strncpy(tmp, index(new_url + 5, ':') + 1, index(new_url + 7, '/') - index(new_url + 5, ':'));
			proxyport = atoi(tmp);//�˿�ת��int
			if (proxyport == 0)
				proxyport = 80;//ȱʡֵ
		}
		else//���û��ָ���˿ں�
		{
			strncpy(host, new_url + 7, index(new_url + 7, '/') - new_url - 7);//hostΪhttp://�ͽ�β��/֮��
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
	//fork֮��Ĵ���
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
	//�ܵ�ͨ��
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