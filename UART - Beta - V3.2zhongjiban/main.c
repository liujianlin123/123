/****************************************************************************
* �� �� ��: main.c
* ��    ��: Amo [ www.amoMcu.com ��Ī��Ƭ��]
* ��    ��: 2014-04-08
* ��    ��: 1.0
* ��    ��: ���ô��ڵ������ֲ����ʣ�115200bps 8N1
*           ���ڵ������ָ�CC254x���ַ���ʱ��������᷵�ؽ��յ����ַ���
****************************************************************************/
#include <ioCC2540.h>
#include <string.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define UART0_RX    1
#define UART0_TX    2
#define SIZE       51

#define LED1 P1_0       // P1.0�ڿ���LED1
//#define KEY1 P0_1       // P0.1�ڿ���S1
#define KEY1 P0_0       // P0.0�ڿ���S1

char RxBuf;
char UartState;
uchar count;
char RxData[SIZE];        //�洢�����ַ���
unsigned char  time,irtime;//������ȫ�ֱ���
unsigned char  num=0;//���ÿ�����ڽ����жϴ���
unsigned char Num=0;//���ÿ�����ڽ����жϴ���
unsigned char flag1=0;//���ÿ�����ڽ����жϴ���
unsigned char flag2=0;//���ÿ�����ڽ����жϴ���
uchar irpro_ok,irok;
unsigned char IRcord[15];
unsigned char irdata[121];
unsigned char irAsc[32];
static unsigned char Tab[17]="0123456789ABCDEF";
/*------------------------------------------------
                  ��������
------------------------------------------------*/
void Ir_work(void);
void Ircordpro(void);
void UartSendHex(unsigned char Data);
void UartSendString(unsigned char *Data, int len);

/****************************************************************************
* ��    ��: DelayMS()
* ��    ��: �Ժ���Ϊ��λ��ʱ
* ��ڲ���: msec ��ʱ������ֵԽ����ʱԽ��
* ���ڲ���: ��
****************************************************************************/
void DelayMS(uint msec)
{ 
    uint i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<1070; j++);
}
/****************************************************************************
* ��    ��: InitLed()
* ��    ��: ����LED����Ӧ��IO��
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void InitLed(void)
{
    P1DIR |= 0x01;   //P1.0����Ϊ�����
    LED1 = 0;        //LED1���� 
}
/****************************************************************************
* ��    ��: InitKey()
* ��    ��: ����KEY��Ӧ��IO��,�����жϷ�ʽ 
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void InitKey()
{
//    P0IEN |= 0x2;    // P0.1 ����Ϊ�жϷ�ʽ 1���ж�ʹ��
  
    P0IEN |= 0x01;    // P0.1 ����Ϊ�жϷ�ʽ 1���ж�ʹ��
    PICTL |= 0x2;    //�½��ش���   
    IEN1 |= 0x20;    //����P0���ж�; 
    P0IFG = 0x00;    //��ʼ���жϱ�־λ
//    EA = 1;          //�����ж�
}
/****************************************************************************
* ��    ��: InitUart()
* ��    ��: ���ڳ�ʼ������
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void InitUart(void)
{ 
    PERCFG = 0x00;           //������ƼĴ��� USART 0��IOλ��:0ΪP0��λ��1 
    P0SEL = 0x0c;            //P0_2,P0_3�������ڣ����蹦�ܣ�
    P2DIR &= ~0xC0;          //P0������ΪUART0
    
    U0CSR |= 0x80;           //����ΪUART��ʽ
    U0GCR |= 11;				       
    U0BAUD |= 216;           //��������Ϊ115200
    UTX0IF = 0;              //UART0 TX�жϱ�־��ʼ��λ0
    U0CSR |= 0x40;           //������� 
    IEN0 |= 0x84;            //�����ж���������ж�  
}
/****************************************************************************
* ��    ��: P0_ISR(void) �жϴ����� 
* ��    ��: #pragma vector = �ж����������������жϴ������
****************************************************************************/
#pragma vector = P0INT_VECTOR    
__interrupt void P0_ISR(void) 
{ 

    static unsigned char  i;             //���պ����źŴ���
    static unsigned char startflag;                //�Ƿ�ʼ�����־λ
//    LED1 = ~LED1;        //LED1���� 
    if(startflag)                         
    {
//                 UartSendHex(irtime);   
      if((irtime>=100))//������ TC9012��ͷ�룬10ms
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
        irdata[i]=irtime;//�洢ÿ����ƽ�ĳ���ʱ�䣬�����Ժ��ж���0����1
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
            irdata[i]=irtime;//�洢ÿ����ƽ�ĳ���ʱ�䣬�����Ժ��ж���0����1
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
    P0IFG = 0;       //���жϱ�־ 
    P0IF = 0;        //���жϱ�־ 
    
} 
/****************************************************************************
* ��    ��: UartSendString()
* ��    ��: ���ڷ��ͺ���
* ��ڲ���: Data:���ͻ�����   len:���ͳ���
* ���ڲ���: ��
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
* ��    ��: UartSendHex()
* ��    ��: ���ڷ��ͺ���
* ��ڲ���: Data:���ͻ�����   len:���ͳ���
* ���ڲ���: ��
****************************************************************************/
void UartSendHex(unsigned char Data)
{

        U0DBUF = Data;
        while(UTX0IF == 0);
        UTX0IF = 0;

}
/****************************************************************************
* ��    ��: UART0_ISR(void) �����жϴ����� 
* ��    ��: ������0���������жϣ����յ������ݱ�����RxBuf��
****************************************************************************/
#pragma vector = URX0_VECTOR 
__interrupt void UART0_ISR(void) 
{ 
    URX0IF = 0;       // ���жϱ�־ 
    RxBuf = U0DBUF;                           
}


/****************************************************************************
* ������ں���
****************************************************************************/
/****************************************************************************
* ��    ��: InitT3()
* ��    ��: ��ʱ����ʼ����ϵͳ�����ù���ʱ��ʱĬ����2��Ƶ����16MHz
* ��ڲ���: ��
* ���ڲ���: ��
****************************************************************************/
void InitT3()
{     
    T3CTL |= 0x08 ;          //������ж�     
    T3IE = 1;                //�����жϺ�T3�ж�
    T3CTL |= 0x60;           //8��Ƶ 62us����һ���ж�//2��Ƶ 16us����һ���ж�
    T3CTL &= ~0x03;          //�Զ���װ 00��>0xff  32us
    T3CTL |= 0x10;           //����
//    EA = 1;                  //�����ж�
}

//��ʱ��T3�жϴ�����
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 


    irtime++;
    IRCON = 0x00;            //���жϱ�־, Ҳ����Ӳ���Զ���� 
    

}
    /*------------------------------------------------
                  ��ֵ����
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
  irpro_ok=0;//������ɱ�־
}
/*------------------------------------------------
                ������ֵ����
------------------------------------------------*/
void Ircordpro(void)//������ֵ������
{ 
    unsigned char i, j,k;
    unsigned char cord,value;
    k=0;
    for(i=0;i<15;i++)      //����15���ֽ�
    {
        for(j=1;j<=8;j++) //����1���ֽ�8λ
        {
            value<<=1;
            cord=irdata[k+1];
            k++;
            //		  SendByte(cord);
            if(cord<0x07)//����ĳֵΪ1������;����о��Թ�ϵ������ʹ��12M���㣬��ֵ������һ�����
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
    irpro_ok=1;//������ϱ�־λ��1
}

void main(void)
{	
    CLKCONCMD &= ~0x40;                        //����ϵͳʱ��ԴΪ32MHZ����
    while(CLKCONSTA & 0x40);                   //�ȴ������ȶ�Ϊ32M
    CLKCONCMD &= ~0x47;                        //����ϵͳ��ʱ��Ƶ��Ϊ32MHZ   
    InitT3();
    InitUart();                                //���ô��ڳ�ʼ������   
    InitLed();   //����LED����Ӧ��IO��
    InitKey();   //����S1��Ӧ��IO�� 
    EA=1;
    while(1)
    {
//        UartSendHex(0xAA);
        if(irok)                        //������պ��˽��к��⴦��
        {   
            Ircordpro();
           irok=0;

        }

        if(irpro_ok)                   //�������ú���й��������簴��Ӧ�İ�������ʾ��Ӧ�����ֵ�
        {
            Ir_work();


        }
    }
        
}
