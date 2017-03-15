/****************************************************************************
* 文 件 名: main.c
* 作    者: Amo [ www.amoMcu.com 阿莫单片机]
* 修    订: 2014-04-08
* 版    本: 1.0
* 描    述: 设置串口调试助手波特率：115200bps 8N1
*           串口调试助手给CC254x发字符串时，开发板会返回接收到的字符串
****************************************************************************/
#include <ioCC2540.h>
#include <string.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define UART0_RX    1
#define UART0_TX    2
#define SIZE       51

#define LED1 P1_0       // P1.0口控制LED1
//#define KEY1 P0_1       // P0.1口控制S1
#define KEY1 P0_0       // P0.0口控制S1

char RxBuf;
char UartState;
uchar count;
char RxData[SIZE];        //存储发送字符串
unsigned char  time,irtime;//红外用全局变量
unsigned char  num=0;//存放每个周期进入中断次数
unsigned char Num=0;//存放每个周期进入中断次数
unsigned char flag1=0;//存放每个周期进入中断次数
unsigned char flag2=0;//存放每个周期进入中断次数
uchar irpro_ok,irok;
unsigned char IRcord[15];
unsigned char irdata[121];
unsigned char irAsc[32];
static unsigned char Tab[17]="0123456789ABCDEF";
/*------------------------------------------------
                  函数声明
------------------------------------------------*/
void Ir_work(void);
void Ircordpro(void);
void UartSendHex(unsigned char Data);
void UartSendString(unsigned char *Data, int len);

/****************************************************************************
* 名    称: DelayMS()
* 功    能: 以毫秒为单位延时
* 入口参数: msec 延时参数，值越大，延时越久
* 出口参数: 无
****************************************************************************/
void DelayMS(uint msec)
{ 
    uint i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<1070; j++);
}
/****************************************************************************
* 名    称: InitLed()
* 功    能: 设置LED灯相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitLed(void)
{
    P1DIR |= 0x01;   //P1.0定义为输出口
    LED1 = 0;        //LED1灯灭 
}
/****************************************************************************
* 名    称: InitKey()
* 功    能: 设置KEY相应的IO口,采用中断方式 
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitKey()
{
//    P0IEN |= 0x2;    // P0.1 设置为中断方式 1：中断使能
  
    P0IEN |= 0x01;    // P0.1 设置为中断方式 1：中断使能
    PICTL |= 0x2;    //下降沿触发   
    IEN1 |= 0x20;    //允许P0口中断; 
    P0IFG = 0x00;    //初始化中断标志位
//    EA = 1;          //打开总中断
}
/****************************************************************************
* 名    称: InitUart()
* 功    能: 串口初始化函数
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitUart(void)
{ 
    PERCFG = 0x00;           //外设控制寄存器 USART 0的IO位置:0为P0口位置1 
    P0SEL = 0x0c;            //P0_2,P0_3用作串口（外设功能）
    P2DIR &= ~0xC0;          //P0优先作为UART0
    
    U0CSR |= 0x80;           //设置为UART方式
    U0GCR |= 11;				       
    U0BAUD |= 216;           //波特率设为115200
    UTX0IF = 0;              //UART0 TX中断标志初始置位0
    U0CSR |= 0x40;           //允许接收 
    IEN0 |= 0x84;            //开总中断允许接收中断  
}
/****************************************************************************
* 名    称: P0_ISR(void) 中断处理函数 
* 描    述: #pragma vector = 中断向量，紧接着是中断处理程序
****************************************************************************/
#pragma vector = P0INT_VECTOR    
__interrupt void P0_ISR(void) 
{ 

    static unsigned char  i;             //接收红外信号处理
    static unsigned char startflag;                //是否开始处理标志位
//    LED1 = ~LED1;        //LED1灯灭 
    if(startflag)                         
    {
//                 UartSendHex(irtime);   
      if((irtime>=100))//引导码 TC9012的头码，10ms
        {
            i=0;
//            ok=1;
            Num=num;
            num=0;
            if(Num<=97&&Num>=95)
            {
            flag2=1;
            }
            if(Num<=121&&Num>=119)
            {
            flag1=1;
            }
//           UartSendHex(Num);   
            Num=0;
            irtime=0;
        }

//        UartSendHex(irdata[i]);
        if(flag1)
        {
        while(KEY1==1)
        irdata[i]=irtime;//存储每个电平的持续时间，用于以后判断是0还是1
        irtime=0;
//        UartSendHex(irdata[i]);
        if(i>=120)
        {
            irok=1; 
            i=0;
//            UartSendString(irdata,15);
        }

        i++;
        }
        if(flag2)
        {
           while(KEY1==1)
            irdata[i]=irtime;//存储每个电平的持续时间，用于以后判断是0还是1
            irtime=0;
//            UartSendHex(irdata[i]);
        if(i>=96)
        {
            irok=1; 
            i=0;
        }
              i++;
        }
    }
    else
    {
        irtime=0;
        startflag=1;
//        ok=0;
    }
    irtime=0;
    num++;
    P0IFG = 0;       //清中断标志 
    P0IF = 0;        //清中断标志 
    
} 
/****************************************************************************
* 名    称: UartSendString()
* 功    能: 串口发送函数
* 入口参数: Data:发送缓冲区   len:发送长度
* 出口参数: 无
****************************************************************************/
void UartSendString(unsigned char *Data, int len)
{
    uint i;
    
    for(i=0; i<len; i++)
    {
        U0DBUF = *Data++;
        while(UTX0IF == 0);
        UTX0IF = 0;
    }
}
/****************************************************************************
* 名    称: UartSendHex()
* 功    能: 串口发送函数
* 入口参数: Data:发送缓冲区   len:发送长度
* 出口参数: 无
****************************************************************************/
void UartSendHex(unsigned char Data)
{

        U0DBUF = Data;
        while(UTX0IF == 0);
        UTX0IF = 0;

}
/****************************************************************************
* 名    称: UART0_ISR(void) 串口中断处理函数 
* 描    述: 当串口0产生接收中断，将收到的数据保存在RxBuf中
****************************************************************************/
#pragma vector = URX0_VECTOR 
__interrupt void UART0_ISR(void) 
{ 
    URX0IF = 0;       // 清中断标志 
    RxBuf = U0DBUF;                           
}


/****************************************************************************
* 程序入口函数
****************************************************************************/
/****************************************************************************
* 名    称: InitT3()
* 功    能: 定时器初始化，系统不配置工作时钟时默认是2分频，即16MHz
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitT3()
{     
    T3CTL |= 0x08 ;          //开溢出中断     
    T3IE = 1;                //开总中断和T3中断
    T3CTL |= 0x60;           //8分频 62us产生一次中断//2分频 16us产生一次中断
    T3CTL &= ~0x03;          //自动重装 00－>0xff  32us
    T3CTL |= 0x10;           //启动
//    EA = 1;                  //开总中断
}

//定时器T3中断处理函数
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 


    irtime++;
    IRCON = 0x00;            //清中断标志, 也可由硬件自动完成 
    

}
    /*------------------------------------------------
                  键值处理
------------------------------------------------*/

void Ir_work(void)
{
  unsigned char i;
  for(i=0;i<15;i++)  
  {
     irAsc[2*i]   =Tab[IRcord[i]/16];
     irAsc[2*i+1] =Tab[IRcord[i]%16];
  }
    irAsc[30]='\r';
    irAsc[31]='\n';
    UartSendString(irAsc,32);     
  irpro_ok=0;//处理完成标志
}
/*------------------------------------------------
                红外码值处理
------------------------------------------------*/
void Ircordpro(void)//红外码值处理函数
{ 
    unsigned char i, j,k;
    unsigned char cord,value;
    k=0;
    for(i=0;i<15;i++)      //处理15个字节
    {
        for(j=1;j<=8;j++) //处理1个字节8位
        {
            value<<=1;
            cord=irdata[k+1];
            k++;
            //		  SendByte(cord);
            if(cord<0x07)//大于某值为1，这个和晶振有绝对关系，这里使用12M计算，此值可以有一定误差
            value|=0x01;
            //			 value&=0x7f;

        }
        IRcord[i]=value;
        value=0; 
//        UartSendHex(IRcord[i]);
//        UartSendHex(0x13);
        
    }
//    UartSendString(IRcord,15); 
//    UartSendHex(0x13);
    irpro_ok=1;//处理完毕标志位置1
}

void main(void)
{	
    CLKCONCMD &= ~0x40;                        //设置系统时钟源为32MHZ晶振
    while(CLKCONSTA & 0x40);                   //等待晶振稳定为32M
    CLKCONCMD &= ~0x47;                        //设置系统主时钟频率为32MHZ   
    InitT3();
    InitUart();                                //调用串口初始化函数   
    InitLed();   //设置LED灯相应的IO口
    InitKey();   //设置S1相应的IO口 
    EA=1;
    while(1)
    {
//        UartSendHex(0xAA);
        if(irok)                        //如果接收好了进行红外处理
        {   
            Ircordpro();
           irok=0;

        }

        if(irpro_ok)                   //如果处理好后进行工作处理，如按对应的按键后显示对应的数字等
        {
            Ir_work();


        }
    }
        
}
