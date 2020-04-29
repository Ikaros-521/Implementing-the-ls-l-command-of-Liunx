#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

#define MAX_FILE_NUM 200


//可能还有一些小问题没有解决，功能基本已经实现，如有建议，望大佬赐教

typedef struct LS
{
	char mode[15];	// 文件的模式
	int dir_num;	// 是否目录或目录中包含目录的数量
	char user[20];	// 文件的用户名
	char group[20];	// 文件的组名
	long size;		// 文件的字节数
	char time[30];	// 文件的最后修改时间
	char year[5];	// 拓展用，年份
	char mon[5];	// 月份
	char hour[5];	// 时
	char min[5];	// 分
	int st_mode;	// 文件类型和权限
	char name[20];	// 文件名
}LS; 

// 获取文件的模式
char* file_mode(mode_t m,char* str)
{
	if(S_ISREG(m))
		str[0] = '-';
	else if(S_ISDIR(m))
		str[0] = 'd';
	else if(S_ISCHR(m))
		str[0] = 'c';
	else if(S_ISBLK(m))
		str[0] = 'b';
	else if(S_ISFIFO(m))
		str[0] = 'q';
	else if(S_ISLNK(m))
		str[0] = 'l';
//	else if(S_ISSOCK(m))
//		str[0] = 's';
	else 
		str[0] = '?';

	str[1] = '\0';

	strcat(str,S_IRUSR&m?"r":"-");
	strcat(str,S_IWUSR&m?"w":"-");
	strcat(str,S_IXUSR&m?"x":"-");

	strcat(str,S_IRGRP&m?"r":"-");
	strcat(str,S_IWGRP&m?"w":"-");
	strcat(str,S_IXGRP&m?"x":"-");
	
	strcat(str,S_IROTH&m?"r":"-");
	strcat(str,S_IWOTH&m?"w":"-");
	strcat(str,S_IXOTH&m?"x":"-");

	return str;
}

// 获取目录的数量
int dir_count(char* path)
{
	DIR *dir;
	dir = opendir(path);
	struct dirent *dirent;
	int count = 0;
	while((dirent = readdir(dir)) != NULL)
	{
		if(dirent->d_type == 4)
			count++;
	}
	closedir(dir);
	return count;
}

// 是否是目录或目录下有目录
int is_dir(struct dirent *dirent)
{
	char* a = dirent->d_name;
	if(dirent->d_type == 8)
		return 1;
	if(dirent->d_type == 4)
	{
		if(dir_count(a) == 0)
			return 2;
		else
			return dir_count(a);
	}
}

// 获取用户名
char* file_user(uid_t st_uid,char* str)
{
	struct passwd *user;
	user = getpwuid(st_uid);
	sprintf(str,"%s",user->pw_name);
	return str;
}

// 获取组名
char* file_group(uid_t st_uid,char* str)
{
	struct passwd *user;
	user = getpwuid(st_uid);
	struct group *grp;
	grp = getgrgid(user->pw_gid);
	sprintf(str,"%s",grp->gr_name);
	return str;
}

// 获取文件大小
off_t file_size(struct stat buf)
{
	off_t size = buf.st_size;
	return size;
}

// 获取最后修改时间
char* file_time(time_t mt,char* str)
{
	struct tm* t = localtime(&mt);
	sprintf(str,"%d月 %02d %02d:%02d",t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min);
	return str;
}

// 获取文件的数量
int file_count(char* path)
{
	DIR *dir;
	dir = opendir(path);
	struct dirent *dirent;
	int count = 0;
	while((dirent = readdir(dir)) != NULL)
	{
		count++;
	}
	closedir(dir);
	return count;
}

// 交换
void equal(LS *a,LS *b)
{
	strcpy(a->mode,b->mode);
	a->dir_num = b->dir_num;
	strcpy(a->user,b->user);
	strcpy(a->group,b->group);
	a->size = b->size;
	strcpy(a->time,b->time);
	a->st_mode = b->st_mode;		
	strcpy(a->name,b->name);
}

// 排序
void sort(LS *info,int index)
{
	LS *temp = (LS*)malloc(sizeof(LS));
	for(int i=index-1; i>0; i--)
	{
		for(int j=0; j<i; j++)
		{
			if(strcmp(info[i].name,info[j].name)<0)
			{
				equal(temp,&info[i]);
				equal(&info[i],&info[j]);
				equal(&info[j],temp);
			}
		}
	}
}

// 输出结构体
void show_ls(LS *info,int index)
{
	for(int i=0; i<index; i++)
	{	
		//printf("%d: ",i);
		printf("%s \033[0m",info[i].mode);
		printf("%d ",info[i].dir_num);
		printf("%s ",info[i].user);
		printf("%s ",info[i].group);
		printf("%5ld ",info[i].size);
		printf(" %s ",info[i].time);
		//printf("%d ",info[i].st_mode);		
		if(16893 == info[i].st_mode)
		{
			// 颜色
			printf("\033[34m\033[1m%s\033[0m",info[i].name);
		}
		else if(33277 == info[i].st_mode)
		{
			printf("\033[32m\033[1m%s\033[0m",info[i].name);
		}
		else
		{
			printf("%s",info[i].name);
		}
		if(i < index)
			printf("\n");
	}
	//printf("循环结束\n");
}

// 创建结构体，赋值
LS *create(struct stat buf,struct dirent *dirent)
{
	LS* info = (LS*)malloc(sizeof(LS));
	char str[50] = {};
	//puts(file_mode(buf.st_mode,str));
	strcpy(info->mode,file_mode(buf.st_mode,str));
	//puts(info->mode);
	info->dir_num = is_dir(dirent);
	strcpy(info->user,file_user(buf.st_uid,str));
	strcpy(info->group,file_group(buf.st_uid,str));
	info->size = file_size(buf);
	strcpy(info->time,file_time(buf.st_mtime,str));
	info->st_mode = buf.st_mode;		
	strcpy(info->name,dirent->d_name);

	return info;
}

int main(int argc,char* argv[])
{
	LS info[MAX_FILE_NUM];
	char* l = "-l";
        if(argc != 2)
        {
            printf("仅支持传入 -l\n");
            return 1;
        }
	if(strcmp(argv[1],l) != 0)
	{
		printf("\"ls：无法识别的选项\"%s\"\n",argv[1]);
		printf("请尝试执行\"ls --help\"来获取更多信息。\n");
		return 0;
	}
	char* a = ".";
	char* b = "..";
	char* path = malloc(10000);
	strcpy(path,"./");	// 只支持当前路径
	int count = file_count(path);

	DIR *dir;
	dir = opendir(path);
	struct dirent *dirent;
	int index = 0;	// 结构体下标
	int blocks = 0;
	for(int i=0; i<count; i++)
	{
		dirent = readdir(dir);
		struct stat buf = {};
		if(stat(dirent->d_name,&buf))
		{
			perror("stat");
			return -1;
		}

		// 跳过特殊情况
		if(strcmp(dirent->d_name,a)==0 || strcmp(dirent->d_name,b)==0)
			continue;
		blocks += buf.st_blocks;
		//printf("%d\n",blocks);
		info[index++] = *create(buf,dirent);
	}
	closedir(dir);
	//printf("文件总数:%d\n",index);
	//show_ls(info,index);	

	printf("总用量 %d\n",blocks/2);
	sort(info,index);
	show_ls(info,index);
	return 0;
}

