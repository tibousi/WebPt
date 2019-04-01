#ifndef Header_All
#define Header_All
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>//获取下载时间
#include <sys/stat.h>//stat系统调用获取文件大小
#include <fcntl.h>//open系统调用
#include <time.h>


int thread_flag = 0;

const char *http_head[] = { "http://", "https://", NULL };

int tty_len;

char *progress_bar;

int url_type = 0;//0表示是测试网站访问速度，1表示测试url下载速度

int flag = 0;
int fileflag = 0;
int httpv = 0;
int cache = 0;
char *request_method[5] = { "GET ", "HEAD ", "OPTIONS ", "TRACE ", "POST " };
char *post_type[4] = {
	"application/x-www-form-urlencoded",
	"multipart/form-data",
	"application/json",
	"text/xml"
};

char url[2048] = { 0 };
char file_name[256] = { 0 };

int file_name_flag = 0;
char file_name_tmp[256] = { 0 };


int postflag = 0;
int posttype = 0;
int method = 0;
int clients = 1;
int testtime = 10;
int proxyport = 80;
char *proxyhost = NULL;
int mypipe[2];
FILE *path = NULL;
int message_length = 0;

char program[256];
char host[1024];
char postdata[1024];
char file_path[1024];
char request[2048];
char new_url[1024];
char message[1024][2048];
char outcome[4096];


int success = 0;
int fail = 0;
int bytes = 0;

struct header_inf{
	int reponse_inf;
	char content_type[128];
	int content_length;
};

void deal_flag(int signal);

void build_request(const char *url);

int deal_pid();

void turn_url(const char *url);

void build_request(const char *url);

void child(const char *host, const int port, const char *request);

void resolve_url(char* url, char* host, int *port, char* file_name);

struct header_inf get_header_inf(char* response);

long get_file_size(char* filename);

void get_ip(char *host_name, char *ip_addr);

int check_server(struct header_inf h);

int Socket_connect(char *url, char *host, char *ip_addr, int port, long file_now, long file_end);

char *find_end(int sock);

void merge_tmp(char *file_path, char *file_name, int num);

void diect();

void download_file(int sock, char* file_name, long content_length);

void download_file_section(int sock, char *file_name, char *url, char *host, char* ip_addr, int port, long file_now, long file_end, int num, int bar_len);

#endif