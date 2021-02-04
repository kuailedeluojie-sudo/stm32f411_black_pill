#include "iap.h"


#include "stmflash.h"

typedef  void (*iapfun)(void);
iapfun jump_app;
uint32_t iapbuf[512]; 	//2K�ֽڻ���

__asm void MSR_MSP(uint32_t addr)
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t appsize)
{
    uint32_t t;
    uint16_t i=0;
    uint32_t temp;
    uint32_t fwaddr=appxaddr;//��ǰд��ĵ�ַ
    uint8_t *dfu=appbuf;
    for(t=0; t<appsize; t+=4)
    {
        temp=(uint32_t)dfu[3]<<24;
        temp|=(uint32_t)dfu[2]<<16;
        temp|=(uint32_t)dfu[1]<<8;
        temp|=(uint32_t)dfu[0];
        dfu+=4;//ƫ��4���ֽ�
        iapbuf[i++]=temp;
        if(i==512)
        {
            i=0;
            STMFLASH_Write(fwaddr,iapbuf,512);
            fwaddr+=2048;//ƫ��2048  512*4=2048
        }
    }
    if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(uint32_t appxaddr)
{
    // �ж�ջ����ֵַ�Ƿ���0x2000 0000 - 0x 2000 2000֮��
    //appxaddr���û�APP����ʼ�ĵ�ַ�����ǻ��APP������ж������������appxaddr��ʼ��λ�ã����ж����������һ���ŵľ���ջ����ַ��ֵ
    //��仰ͨ���ж�ջ����ֵַ�Ƿ���ȷ���Ƿ���0x2000 0000 - 0x 2000 2000֮�䣩
    //��������Ӧ�ó���������ļ��տ�ʼ�ͻ�ȥ��ʼ��ջ�ռ䣬���ջ��ֵ���ˣ�˵��Ӧ�ó��Ѿ������ˣ������ļ��ĳ�ʼ��Ҳִ���ˡ�
    if(((*(__IO uint32_t*)appxaddr)&0x2FF00000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
    {
        // (vu32* )(appxaddr+4)����ŵ����ж�������ĵڶ����λ��ַ��
			 //iapfun�ı���������Ϊtypedef void (* iapfun)(void); �� iapfun������ void (*)(void)����ָ�� ��һ������
			//���Դ�ʱ��jump2appָ��APP��λ�������ڵĵ�ַ
        jump_app=(iapfun)*(__IO uint32_t*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        MSR_MSP(*(__IO uint32_t*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
			//	__set_MSP(*(__IO uint32_t*)(appxaddr));	
			printf("%s %d \r\n",__FUNCTION__,__LINE__);  
			jump_app();									//��ת��APP.
    }
}

