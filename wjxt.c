#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
//linux下不用注释 
//#include<sys/io.h> 
#include<fcntl.h>
#include <unistd.h>

//用于加锁
pthread_mutex_t lock;

//结构体，用于存储用户输入的命令及文件名
typedef struct{
	
	int number;		//用于不同线程标示，表示不同的物理地址对control模块的访问
	char cmd[100];
	char filename[100]; 
	
}MESSAGE;

//结构体用于存储用户输入的更改文件的起终地址，需要修改的信息
typedef struct{
	
	int ioname;						//定义线程名称	
	char start_termination[100];	//定义字符数组存储更改信息的起终地址
	char update_content[100];		//定义字符数组存储用户要修改的内容	
	char update_filename[100];		//定义字符数组存储用户要修改的文件名 
	char update_newfilename[100];	//定义字符数组存储用户修改后的文件名 
	
}UPDATEMESSAGE;

//默认文件不存在 
int flag = -1;		//定义flag变量判断用户更新的是内容还是文件名 

//control中用于接收指令receive
char r_cmd1[100];
char r_cmd2[100];

//control中用于接收文件名
char r_filename1[100];
char r_filename2[100];

//定义二维文件目录数组，横向用于存储创建的文件名
char filename[100][50];

//定义三个字符串用于判断用户输入的字符
char c[100]="create";
char d[100]="delete";
char u[100]="update";
char e[6]="exit";

//用户输入模块
void* iofunc(void* args){

	pthread_mutex_lock(&lock);				//加锁 
	MESSAGE* message = (MESSAGE*) args;		//强制转换接收到的数据类型 
	int num = message->number;				//使用num接收线程号

	while(1){
		printf("******************************\n");
		printf("        创建文件：create\n");
		printf("        删除文件：delete\n");
		printf("        修改文件：update\n");
		printf("        退出系统：exit\n");
		printf("******************************\n");
		printf("你好 %d 号用户，请输入你要进行的操作：",num);

		//用户输入命令
		scanf("%s",message->cmd);

		//校验用户输入的指令，strcmp函数用于比较输入的cmd和命令
		if(strcmp(message->cmd,c)==0||strcmp(message->cmd,d)==0||strcmp(message->cmd,u)==0){
			printf("输入正确\n");
			break;
		}
		else if(strcmp(message->cmd,e)==0){
			exit(0);
		}else{
			printf("输入错误，请重新输入！\n");
			continue;
		}
	}
	printf("请输入增加/删除/更改的文件名:");
	scanf("%s",message->filename);

	//解锁
	pthread_mutex_unlock(&lock);
	return NULL;
}

//创建文件函数
/*这里使用数组对输入的文件名进行存储，也可以使用目录.txt存储文件名称*/
void* createfile(void* args){
	printf("\n");
	 
	MESSAGE* message = (MESSAGE*) args;		//强制转换接收到的数据类型 		
	int num = message->number;				//定义整型变量num接收线程号

	//用access函数判断文件存在性 
	if((access(message->filename,F_OK))!=-1)
    {
        printf("你好，%d号用户，文件已存在!\n", num);
    }else{
		printf("你好，%d号用户，正在为您创建......", num);
		
		//creat函数中0777表示用户对创建的文件可执行读写操作
		int fd = creat(message->filename,0777);
		if(fd == -1){
			printf("创建失败！\n");
		}else{
			int i; 
			for(i=0;i<100;i++){
				if(filename[i][0]=='\0'){
					strcpy(filename[i],message->filename);
					printf("创建成功！\n");
					break; 
					close(fd);
				}else{
					continue;
				}
			}
		} 
	} 
	return 0;
}

//删除文件函数
void* deletefile(void* args){
	printf("\n");
	
	//强制转换接收到的数据类型
	MESSAGE* message = (MESSAGE*) args;

	//用于接收线程号
	int num = message->number;
	
	int choose;
	//access函数判断文件存在性 
	if((access(message->filename,F_OK))!=-1){
		
		printf("你好，%d号用户请输入你的选择：\n",num);
		printf("删除文件请输入1；删除文件内容请输入2：\n");	
    	scanf("%d",&choose);
	    	
    	if(choose==1){
    		printf("你好，%d号用户，该文件存在，正在为您删除......", num);
			remove(message->filename);
			printf("删除成功！\n",num);
		}
        else if(choose==2){
			printf("%s内容为：\n",message->filename);
			FILE *fp = fopen(message->filename, "r");	//执行读操作 
			char str[100];
			int c,i;
			for(i=0;i<100;i++){
				if((c=fgetc(fp))!=EOF){
					str[i]=c;
					putchar(c);
				} 
			}
			printf("\n");
			printf("请输入你要删除的第几个字符：");
			int ch;
			scanf("%d",&ch);
			for(i=ch-1;i<100;i++){
			str[i]=str[i+1];
			}
			fclose(fp); 
			//将旧文件删除 
			remove(message->filename);
				
			//creat函数中0777表示用户对创建的文件可执行读写操作
			int fd = creat(message->filename,0777);	
						
			printf("删除成功！删除后的文件内容为：");	
			puts(str);				
			
			//创建新文件存储旧文件中剩余信息，文件名不变 
			FILE *ff = fopen(message->filename, "w");
			fputs(str, ff); 
		}
    }
	else
	{
		printf("你好，%d号用户，文件不存在！\n", num);
	}
}
//函数用于接收用户输入的修改信息 
void* updatefunc(void* args){
	printf("\n");
	
	//初始化control模块传过来的updatemessage
	UPDATEMESSAGE* func_updatemessage = (UPDATEMESSAGE*) args;	
	
	int num = func_updatemessage->ioname;		//用num接收线程名称
	//access函数判断文件存在性 
	if((access(func_updatemessage->update_filename,F_OK))!=-1){
		
		printf("你好，%d号用户请输入你的选择：\n",num);
		printf("修改文件名请输入1；修改文件内容请输入2：\n");	
		scanf("%d",&num);
		
		if(num == 1){
			printf("%d号用户，请输入您修改后的文件名：", num);
			scanf("%s", func_updatemessage->update_newfilename); 
			
			flag = 0; 	//判断出用户要修改文件名，flag置为0 
			
		}else if(num == 2){
			printf("%d号用户，请输入需要更改的起始地址和终止地址，输入格式：起始位置_终止位置:",num);
			scanf("%s", func_updatemessage->start_termination);
			
			flag = 1;	//判断出用户要修改文件内容，flag置为1  		
		
			while(1){
				printf("请输入更改后的信息(长度不超过10个字母）:");
				scanf("%s",func_updatemessage->update_content);
			
				//strlen函数用于获取用户输入信息的长度
				if(strlen(func_updatemessage->update_content)<=10){
					break;
				}else{
					printf("输入信息过长！");
				}
			} 
		}
	}
	else{
		printf("你好，%d号用户，文件不存在！\n", num);
	} 
	return NULL;
}

//函数用于实现修改文件名或文件内容操作 
void* updatefile(void* args){
	
	//用updatefile_updatemessage接收control传过来的用户更改信息
	UPDATEMESSAGE* updatefile_updatemessage = (UPDATEMESSAGE*) args;
	
	//用num接收updateflile传来的线程名称
	int num = updatefile_updatemessage->ioname;
	
	char s_t[100];				//定义字符数组s_t存储	
	char u_c[100];				//定义字符数组u_c存储更改后的信息
	char u_filename[100];		//定义字符数组存储要修改文件名
	char u_newfilename[100];	//定义字符数组存储修改后的文件名 
	
	//判断用户要修改文件名还是文件内容 
	if(flag == 0){
		//将接收到用户要修改的文件名存储到更UPDATEMESSAGE结构体的相应数组中 
		strcpy(u_filename,updatefile_updatemessage->update_filename);
		//测试	printf("要修改的文件名输出：%s", u_filename); 
		
		//将接收到用户修改后的文件名存储到更UPDATEMESSAGE结构体的相应数组中 
		strcpy(u_newfilename, updatefile_updatemessage->update_newfilename);
		//测试	printf("要修改的文件名输出：%s", u_newfilename); 
		
		//更改文件名 
		rename(u_filename, u_newfilename); 
		//测试	printf("修改后的文件名输出：%s", u_newfilename); 
		
		printf("你好，%d号用户，文件名修改完成！\n",num);
		
	}else if(flag == 1){
		
		//将接收到的起终地址信息存储到UPDATEMESSAGE结构体的相应数组中 
		strcpy(u_filename,updatefile_updatemessage->update_filename);
		strcpy(s_t,updatefile_updatemessage->start_termination);
		strcpy(u_c,updatefile_updatemessage->update_content);		
		//测试	printf("起终数组输出：%s",s_t);
		//测试	printf("更改信息输出：%s\n",u_c);
	
		int start=0;
		int termination=0;

		sscanf(updatefile_updatemessage->start_termination,"%d_%d",&start,&termination);

		FILE *fp;
		
		//测试	printf("这里是修改文件函数，这里的update_filename是：%s",u_filename);
		fp=fopen(u_filename,"r+");

		//使用fseek函数将文件指针移到更改信息的终止位置	
		fseek(fp,termination*1L,0);		
		
		char working_area[500];		//定义缓冲数组	
		
		//使用fscanf函数将文件终止位置后的函数输入到缓冲数组里	
		fscanf(fp,"%s",working_area);			
		//测试	printf("缓冲数组中的数据是：%s",working_area);	
			
		//将指针移动到更改信息起始位置	
		fseek(fp,start*1L,0);
		
		//使用fprintf函数将更改信息u_c写入到文件中						
		fprintf(fp,"%s",u_c);	
			
		//将指针移动到 起始位置+更改后的信息的长度处			
		fseek(fp,(sizeof(u_c) + start)*1L,0);
		
		//将缓冲数组中的信息写入文件			
		fprintf(fp,"%s",working_area);			

		fclose(fp);	
		printf("你好，%d号用户，文件内容修改完成！\n",num);
	}
	return NULL;
}

int main()
{
	//创建两个线程
	pthread_t io1;
	pthread_t io2;

	//创建线程cre, del, upd1分别用于创建、删除、更改
	pthread_t cre;
	pthread_t del;
	pthread_t upd1;
	
	//创建线程upd2用于接收修改后的文件内容
	pthread_t upd2;	
	
	//动态方式创建互斥锁
	pthread_mutex_init(&lock, NULL);	

	//初始化用户输入的指令和文件名
	MESSAGE message1 = {1, 0, 0};
	MESSAGE message2 = {2, 0, 0};

	//创建两个线程，实现多线程通信
	pthread_create(&io1, NULL, iofunc, &message1);
	pthread_create(&io2, NULL, iofunc, &message2);
	
	//用于等待一个线程的结束
	pthread_join(io1, NULL);
	pthread_join(io2, NULL);

	//初始化更改文件的起终位置，更换信息，调用线称号
	UPDATEMESSAGE updatemessage1 = {1, 0, 0, 0};
	UPDATEMESSAGE updatemessage2 = {2, 0, 0, 0};

	strcpy(updatemessage1.update_filename, message1.filename);
	strcpy(updatemessage2.update_filename, message2.filename);

	//线程1部分
	if(strcmp(message1.cmd, c) == 0){
		//测试	printf("程序进入if，create函数\n");
		pthread_create(&cre, NULL, createfile, &message1);
		pthread_join(cre, NULL);
	}
	else if(strcmp(message1.cmd, d) == 0){
		//测试	printf("程序进入if，delete函数即将进入线程\n");
		pthread_create(&del, NULL, deletefile, &message1);
		pthread_join(del, NULL);
	}	
	else if(strcmp(message1.cmd, u) == 0){
		//测试	printf("程序进入if，update函数\n");
		pthread_create(&upd2, NULL, updatefunc, &updatemessage1);
		pthread_join(upd2, NULL);
		//测试	printf("主函数中的start_termination是：%s\n",update);
		pthread_create(&upd1, NULL, updatefile, &updatemessage1 );
		pthread_join(upd1,NULL);
	}	

	//线程2部分
	if(strcmp(message2.cmd, c) == 0){
		
		pthread_create(&cre, NULL, createfile, &message2);
		pthread_join(cre, NULL);
	}
	else if(strcmp(message2.cmd, d) == 0){
		
		pthread_create(&del, NULL, deletefile, &message2);
		pthread_join(del, NULL);
	}
	else if(strcmp(message2.cmd, u) == 0){
		
		pthread_create(&upd2, NULL, updatefunc, &updatemessage2);
		pthread_join(upd2, NULL);

		pthread_create(&upd1, NULL, updatefile, &updatemessage2);
		pthread_join(upd1, NULL);
	}
	return 0;
}
