
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#define MAXLINE 512


#include <sys/types.h>  

#include <sys/stat.h>  
#include <fcntl.h>  
#include <termios.h>  
#include <stdio.h>  
#define BAUDRATE        B9600  
#define UART_DEVICE     "/dev/tty"  //串口，待修改

#define FALSE  -1  
#define TRUE   0  


//added by wxn on 2080401
#include "include/CL1306.h"
#include "include/gpio.h"
#include "include/player.h"
#include "include/Read.h"
#include "include/net.h"
#include "include/psam.h"
#include "include/display.h"
#include "include/keyboard.h"
int beep_fd;
char servInetAddr[20] ={0};
char  buf[255];  
char *path="/mnt/nand1-2/app/lengku.ini";
char *savePath="/mnt/nand1-2/app/backup.txt";
char src[10];
char cardId[50];
char currentWorkId[50]={0};
char cardmode[10]="00";
char recvbuf[32];
char dangqiancardid[50];
int maoliaook=0;
char maoliaohebanchengpin[255]={0};
char shijianchuo[10]={0};


char bcardId[50];
char banchengpinweight[6];


//拼接字符串和数字
//added by wxn on 20180405
int cardok=0;
int pinzhongmode=0;

int write_block_m1(int secter,int block,unsigned char *buf)
{
	unsigned char len;	
	int i;
	struct card_buf temkey;
	int in;

	temkey.mode=KEYA;
	memset(temkey.rwbuf,0xff,16);
	memset(temkey.money,0,4);
	DBG_PRINTF("%s\n",__func__);
	memset(temkey.key,0xff,6);

	int ret;
	char csn[32];
	int csnLen;
	
	while(1){

			ret=Get_KeyCode();
			if(ret==4)
				return -1;
			if((ret=CardReset(csn,&csnLen))== 0x08)
				break;
	}
	
	if(WriteOneSertorDataToCard(buf,16,secter,block,WRITE_KEY,temkey.key,temkey.mode)==MI_OK)
		{
			printf("wrie card success============================== \n");
			return 0;
		}
	else{
			buzz_off();
			return -1;
		}
	
}


int read_block_m1(int secter,int block,unsigned char *buf)				//do not need to verify the key
{
	unsigned char secbuff[16];
	unsigned char len;	
	int i;
	struct card_buf temkey;
//	int secter=5;
//	int block=1;
	int in;
	
	temkey.mode=KEYA;
	memset(temkey.rwbuf,0xff,16);
	memset(temkey.money,0,4);
	DBG_PRINTF("%s\n",__func__);
	memset(temkey.key,0xff,6);

	int ret;
	char csn[32];
	int csnLen;

	while(1)
	{
		printf("----------------\n");

		ret=Get_KeyCode();
		if(ret==4)
			return -1;
		if((ret=CardReset(csn,&csnLen))== 0x08)
		{
			break;
		}
	}


	if(ReadOneSectorDataFromCard(buf,&len,secter,block,WRITE_KEY,temkey.key,temkey.mode)==MI_OK)
	{

		printf("read sector len=%d\n",len);

	}
	else 
	{
		buzz_off();
		return -1;
	}
	return 0;
}


int verify_kind()
{
	unsigned char buf[16];
	int i;
	read_block_m1(0,2,buf);
	if((0xff==buf[0])&&(0x00==buf[1])&&(0xff==buf[2]))
	{
		printf("\n this is pin zhong ka\n");
		cardmode[0]=buf[3];
		cardmode[1]=buf[4];
		cardmode[2]='\0';
		TextOut(30,20,"品种:",GB2312_32);
		TextOut(110,20,cardmode,GB2312_32);
		return 1;

	}
	else
	{
	 return 0;
	}
}


//added by wxn on 20180429
int shijianchuofunction()
{
	char * t;
	time_t time2;
	int i=0;
	struct tm *tt;
	time(&time2);
	tt=localtime(&time2);
	t = asctime(tt);

	for(i=0;i<8;i++)
	{
	    shijianchuo[i]=t[11+i];
	}
}

int dangqianshijiancuo()
{
	time_t t;
	int j;
	j = time(&t);
	sprintf(shijianchuo,"%d",j);
	return 0;
}





////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief  设置串口通信速率 
*@param  fd     类型 int  打开串口的文件句柄 
*@param  speed  类型 int  串口速度 
*@return  void 
*/  
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,  
               B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };  
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,   
              115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };  
void set_speed(int fd, int speed)
{  
	int   i;   
	int   status;   
	struct termios   Opt;  
	tcgetattr(fd, &Opt);   
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) 
	{   
		if  (speed == name_arr[i]) {       
			tcflush(fd, TCIOFLUSH);       
			cfsetispeed(&Opt, speed_arr[i]);    
			cfsetospeed(&Opt, speed_arr[i]);     
			status = tcsetattr(fd, TCSANOW, &Opt);    
			if  (status != 0) 
			{          
				perror("tcsetattr fd1");    
				return;       
			}      
			tcflush(fd,TCIOFLUSH);     
		}    
	}  
}  
////////////////////////////////////////////////////////////////////////////////  
/** 
*@brief   设置串口数据位，停止位和效验位 
*@param  fd     类型  int  打开的串口文件句柄 
*@param  databits 类型  int 数据位   取值 为 7 或者8 
*@param  stopbits 类型  int 停止位   取值为 1 或者2 
*@param  parity  类型  int  效验类型 取值为N,E,O,,S 
*/  
int set_Parity(int fd,int databits,int stopbits,int parity)  
{   
	struct termios options;   
	if  ( tcgetattr( fd,&options)  !=  0) {   
		perror("SetupSerial 1");       
		return(FALSE);    
	}  
	options.c_cflag &= ~CSIZE;   
	switch (databits) /*设置数据位数*/  
	{     
		case 7:       
			options.c_cflag |= CS7;   
			break;  
		case 8:       
			options.c_cflag |= CS8;  
			break;     
		default:      
			fprintf(stderr,"Unsupported data size\n"); return (FALSE);    
	}  
	switch (parity)   
	{     
		case 'n':  
		case 'N':      
			options.c_cflag &= ~PARENB;   /* Clear parity enable */  
			options.c_iflag &= ~INPCK;     /* Enable parity checking */   
			break;    
		case 'o':     
		case 'O':       
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/    
			options.c_iflag |= INPCK;             /* Disnable parity checking */   
			break;    
		case 'e':    
		case 'E':     
			options.c_cflag |= PARENB;     /* Enable parity */      
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/       
			options.c_iflag |= INPCK;       /* Disnable parity checking */  
			break;  
		case 'S':   
		case 's':  /*as no parity*/     
			options.c_cflag &= ~PARENB;  
			options.c_cflag &= ~CSTOPB;break;    
		default:     
			fprintf(stderr,"Unsupported parity\n");      
			return (FALSE);    
	}    
	/* 设置停止位*/    
	switch (stopbits)  
	{     
		case 1:      
			options.c_cflag &= ~CSTOPB;    
			break;    
		case 2:      
			options.c_cflag |= CSTOPB;    
			break;  
		default:      
			fprintf(stderr,"Unsupported stop bits\n");    
			return (FALSE);   
	}   
	/* Set input parity option */   
	if (parity != 'n')     
		options.c_iflag |= INPCK;   
	tcflush(fd,TCIFLUSH);  
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/     
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */  
	if (tcsetattr(fd,TCSANOW,&options) != 0)     
	{   
		perror("SetupSerial 3");     
		return (FALSE);    
	}   
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/  
	options.c_oflag  &= ~OPOST;   /*Output*/  
	return (TRUE);    
}  
////////////////////////////////////////////////////////////////////////////////  
int readNum()
{
	int fd, c=0, res;  
	int i=0;
	fd = open(UART_DEVICE, O_RDWR);  


		if (fd < 0) {  
			return -1;
		}  


		printf("Open...\n");  
		set_speed(fd,9600);  
		while (set_Parity(fd,7,1,'O') == FALSE)  {  
			printf("Set Parity Error\n");  
		}  

	printf("Reading...\n");  
	while(1) {  
		res = read(fd, buf, 100); 
		break; 
	}  

	printf("Close...\n");  
	close(fd);  

	return 0;  
}  

int display_char()
{
    int i=0;
    int j=0;
    char kind[3] = "";
    char id[4]  = "";
    Clear_Display();
    while(i<2)
    {
        kind[i]=maoliaohebanchengpin[i+11];
        i++;
    }
    kind[i]='\0';

    while(j<3)
    {
       id[j]=maoliaohebanchengpin[j];
        j++;
    }
    id[j]='\0';
     TextOut(25,20,"品种:",GB2312_32);
     TextOut(95,20,kind,GB2312_32);
     TextOut(90,90,"请刷卡",GB2312_32);
     TextOut(175,20,"工号:",GB2312_32);
     TextOut(255,20,id,GB2312_32);

}


int leiji()
{
    char *p;
    char *p2=" ";
    int i=0;
    int j=0;
    char workid[16]={0};
    char last_work[16]={0};
    char now_work[16]={0};
    char now_time[12]={0};
    char last_time[12]={0};
    int res;
    pid_t pid;
    pid=fork();
    if(0 == pid)
    {
        int socketfd;
        char buf1[32];
        struct sockaddr_in sockaddr;
        socketfd = socket(AF_INET,SOCK_STREAM,0);
        memset(&sockaddr,0,sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(6666);
        inet_pton(AF_INET,servInetAddr,&sockaddr.sin_addr);
        while(1)
        {
            //判断是否链接
            if(0 != connect(socketfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr)))
            {
                printf("connect error %s errno: %d\n",strerror(errno),errno);
                sleep(10);
                continue;
            }
            printf("---line 576 connect success\n");
            while(1)
            {
                sleep(1);
                memset(&buf1,0,sizeof(char)*32);
                memset(recvbuf,0,sizeof(char)*32);
                FILE *fp=fopen(savePath,"ab+");
                fgets(buf1,32,fp);
                fclose(fp);  
                if(strlen(buf1)==30)
                {
                    while(1)
                    {
                        sleep(1);
                        if((send(socketfd,buf1,strlen(buf1),0)) < 0)
                        {
                            printf("send mes error: %s errno : %d",strerror(errno),errno);
                            continue;
                        }
                        if(recv(socketfd,recvbuf,sizeof(recvbuf),0) >0)
                        {
                            if(recvbuf[0]=='0')
                            {
                                system("/mnt/nand1-2/app/del /mnt/nand1-2/app/backup.txt");
                                break;
                            }
                        }
                    }
                }
                else
                {
                    system("/mnt/nand1-2/app/del /mnt/nand1-2/app/backup.txt");
                }
            }
        }
		return 0;
	}
	else
	{

		while(1)
		{
			//先清空，显示请刷卡
			Clear_Display();
			TextOut(25,20,"品种:",GB2312_32);
			TextOut(95,20,cardmode,GB2312_32);
			TextOut(90,90,"请刷卡",GB2312_32);
			TextOut(175,20,"工号:",GB2312_32);
			TextOut(255,20,workid,GB2312_32);

			if(1==verify_kind())
			{
				printf("\n this is pinzhongka\n");
				memset(last_work,0,sizeof(char)*16);
				//品种卡
			}
			else
			{
				memset(cardId,0,sizeof(char)*50);
				memset(buf,0,sizeof(char)*255);
				memset(maoliaohebanchengpin,0,sizeof(char)*255);
				memset(workid,0,sizeof(char)*16);
				while(read_block_m1(0,0,now_work)!=0);

				for(i=0;i<16;i++)
				{
					if(now_work[i]!=last_work[i])
						break;
				}

				if(16 == i)
				{
					time_t t;
					int j;
					j = time(&t);
					sprintf(now_time,"%d",j);

					if(now_time-last_time<60)
					{
						Clear_Display();
						TextOut(50,90,"已经成功刷卡",GB2312_32);
						usleep(300000);
						continue;
					}
					else
						i=0;
				}

				while(read_block_m1(0,1,workid)!=0);
				for(i=0;i<3;i++)
				{
					maoliaohebanchengpin[i]=workid[i];
				}
				maoliaohebanchengpin[3]=',';
				for(i=0;i<6;i++)
				{
					maoliaohebanchengpin[i+4]='0';
				}
				maoliaohebanchengpin[10]=',';
				maoliaohebanchengpin[11]=cardmode[0];
				maoliaohebanchengpin[12]=cardmode[1];
				maoliaohebanchengpin[13]=',';

				while(1)
				{
					while(readNum()!=0); 
					p=strstr(buf,p2);
					//p=strstr(buf,p2);
					//char test2[16]=" 00001531";
					//p=strstr(test2,p2);
					if((p!=NULL)&&(strlen(p)>7))
					{
						p++;
						for(j=0;j<6;j++)
						{
							if((*(p+j)<'0')||(*(p+j)>'9'))
							{

								maoliaohebanchengpin[j+14]='0';   

							}
							else
								maoliaohebanchengpin[j+14]=*(p+j);   
						}
						break;
					}
				}
				maoliaohebanchengpin[19]='0';
				maoliaohebanchengpin[20]=',';
				shijianchuofunction();
				for(i=0;i<8;i++)
				{
					maoliaohebanchengpin[21+i]=shijianchuo[i];
				}
				maoliaohebanchengpin[30]='\0';
				if(strlen(maoliaohebanchengpin)!=29)
				{
					Clear_Display();
					TextOut(70,50,"刷卡失败",GB2312_32);
					TextOut(20,90,"请重新刷卡",GB2312_32);
					usleep(5000);
					continue;

				}
				FILE *fp=fopen(savePath,"ab+");
				while(fprintf(fp,"%s\n",maoliaohebanchengpin)!=30);
				fclose(fp);
				sync();
				Clear_Display();
				buzz_on();
				TextOut(70,50,"刷卡成功",GB2312_32); 
				TextOut(50,90,"工号:",GB2312_32);
				TextOut(130,90,workid,GB2312_32);
				for(i=0;i<16;i++)
				{
					last_work[i]=now_work[i];
				}
				for(i=0;i<11;i++)
				{
					last_time[i]=now_time[i];
				}
				usleep(250000);
				buzz_off();
			}
		}
	}
}




int zhika()
{
	int socketfd;
	char write[16];
	char cardId[20];
	int cardok=0;
	char pinzhongkanum[3]={0};
	char yuangongkanum[4]={0};
    struct sockaddr_in sockaddr;
    socketfd = socket(AF_INET,SOCK_STREAM,0);
    memset(&sockaddr,0,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(6667);
    inet_pton(AF_INET,servInetAddr,&sockaddr.sin_addr);
    while(1)
    {
        //判断是否链接
        if(0 != connect(socketfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr)))
        {
            printf("connect error %s errno: %d\n",strerror(errno),errno);
            continue;
        }

        while(1)
        {
            memset(cardId,0,sizeof(char)*16);
            memset(recvbuf,0,sizeof(char)*16);
            read_block_m1(0,0,cardId);
            cardId[16]='\0';
            printf("----757 read block 0 1\n");
            if((send(socketfd,cardId,strlen(cardId),0)) < 0)
            {
                printf("send mes error: %s errno : %d",strerror(errno),errno);
                continue;
            }
            if(recv(socketfd,recvbuf,sizeof(recvbuf),0) <0)
            {
                printf("---- recv buf %s/n",recvbuf);
                break;
            }

            //品种卡
            if(recvbuf[0]==0x30)
            {
                write[0]=0xff;
                write[1]=0x00;
                write[2]=0xff;
                write[3]=recvbuf[2];
                write[4]=recvbuf[3];

                if(0 == write_block_m1(0,2,write))
                {
                    //显示刷卡成功
                    pinzhongkanum[0]=recvbuf[2];
                    pinzhongkanum[1]=recvbuf[3];
                    pinzhongkanum[2]='\0';
                    printf("\n write success----------\n");
                    Clear_Display();
                    TextOut(70,50,"制作品种卡成功",GB2312_32);
                    TextOut(70,90,"品种卡卡号：",GB2312_32);
                    TextOut(150,130,pinzhongkanum,GB2312_32);
		      usleep(500000);

                }
            }

            //员工卡
            else if(recvbuf[0]==0x31)
            {
                write[0]=recvbuf[2];
                write[1]=recvbuf[3];
                write[2]=recvbuf[4];

                if(0 == write_block_m1(0,1,write))
                {
                    //显示刷卡成功
                    yuangongkanum[0]=recvbuf[2];
                    yuangongkanum[1]=recvbuf[3];
                    yuangongkanum[2]=recvbuf[4];
                    yuangongkanum[3]='\0';
                    printf("\n write success----------\n");
                    Clear_Display();
                    TextOut(70,50,"制作员工卡成功",GB2312_32);
                    TextOut(70,90,"员工卡卡号：",GB2312_32);
                    TextOut(150,130,yuangongkanum,GB2312_32);
		      usleep(500000);
                }
            }
        }
    }
}

unsigned char InitSystem(void)
{
	//开启蜂鸣器
	beep_fd=open("/dev/beep",O_RDWR);
	 if(beep_fd<0)
	 {
		 close(beep_fd);
		 printf("Can't open /dev/beepgpio\n");
		 return -2;
	 }
}  
int main()
{
	
    FILE *fd;
	if(Init_MF("/dev/typea")!=MI_OK)
	{
		exit(1);
	}

	InitSystem(); 
	signal(SIGPIPE, SIG_IGN);  //关闭SIGPIPE信号，防死机
	fd=fopen(path,"r");
	fgets(servInetAddr,20,fd);
	close(fd);
    if(servInetAddr[strlen(servInetAddr)-1]=='\n')
    {
        servInetAddr[strlen(servInetAddr)-1]='\0';
    }
    printf("ip is %s\n",servInetAddr);

  
	if(Open_Frambuffer( "/dev/fb0")==0)
	{
		//Init_Hzk();
		//Set_Font_Color(Color_white);
	
		Set_Background(NULL,Color_yellow,0);
		
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk16c_ASC.DZK",GB2312_16,HZK_MIXUER)==0)
			printf("加载字库 16 成功\n");
		if(Insert_Hzk("/mnt/nand1-2/app/font/hzk32c_ASC.DZK",GB2312_32,HZK_MIXUER)==0)
			printf("加载字库 32 成功\n");
		printf("-----------------\n");

	}
	else{
		printf("打开frambuffer失败\n");
		return -1;
	}

	TextOut(30,90,"系统初始化",GB2312_32);

    Clear_Display();
	TextOut(70,50,"累计模式",GB2312_32);
	TextOut(70,130,"卡模式",GB2312_32);
	int Loop=1;
	int ret;
	int mode;
    if(Open_KeyBoard("/dev/input/event0")==0)
    {
       while(Loop) 
        {
            ret=Get_KeyCode();
            switch(ret)
            {
                case 1:
                mode=1;
                Loop=0;
                break;
             //   case 2:
             //   mode=2;
             //   Loop=0;
            //    break;
                case 3:
                mode=3;
                Loop=0;
                break;
               // case 4:
              //  mode=4;
              //  Loop=0;
             //   break;
 
                default:
                break;
            }
        }
    }
    printf("ret is %d\n",mode);

	if(1==ret)
	{
		leiji();
	}
//	if(2==ret)
//	{
//		banchengpin();
//	}
	if(3==ret)
	{
		zhika();
	}
//	if(4==ret)
//	{

//	}
	return 0;
	
}
