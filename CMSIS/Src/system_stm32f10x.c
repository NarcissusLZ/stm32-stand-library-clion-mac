/******************************************************************************
  * @file    system_stm32f10x.c
  * @author  ħŮ�������Ŷ�
  * @version 
  * @date    2020��4��21��
  * @note    ԭ�����ļ��ֳ��ֆ��£��������ۣ� ��д��
******************************************************************************/
#include "../Inc/stm32f10x.h"



// �ж�������ƫ��
#define VECT_TAB_OFFSET    0x0                                                       // �����޸ģ� �������Ѻ���ϤSTM32���������ж�
// ��ƵԤ����ֵ�Ա�
__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};    // �����޸ģ� ���ڼ��ϵͳʱ��Ƶ��
// ϵͳʱ��Ƶ��
uint32_t SystemCoreClock = 5201314;                                                  // ������������ֵ���Ա����������ʱ���Ƿ�ɹ����ó�: 72'000'000Hz



/*****************************************************************************
*��  ���� system_ClockInit
*��  �ܣ� ϵͳʱ������
*��  ���� ��
*����ֵ�� ��
*��  ע�� ע��_ħŮ������ 
*****************************************************************************/
void SystemInit(void)
{
    // ��λʱ����Ĭ��״̬
    RCC->CR   |= (u32)0x00000001;                                // �����ڲ�ʱ��
    RCC->CFGR &= (u32)0xF8FF0000;                                // ��λ SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits
    RCC->CFGR &= (u32)0xFF80FFFF;                                // ��λ PLL�ͷ�Ƶ������
    RCC->CIR   = 0x009F0000;                                     // �ر��жϣ������ж�λ

    // ����ʱ�� HCLK, PCLK, PCLK1, PCLK2, FLASH 
    __IO u32 StartUpCounter = 0, HSEStatus = 0;      
    
    RCC->CR |= ((u32)RCC_CR_HSEON);                              // ʹ�� HSE  
    do{                                                          // �ȴ�HSE����
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;  
      } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));   // 0x0500

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)  {  
        FLASH->ACR |= FLASH_ACR_PRFTBE;                          // Enable Prefetch Buffer */    
        FLASH->ACR &= (u32)((u32)~FLASH_ACR_LATENCY);            // Flash 2 wait state */
        FLASH->ACR |= (u32)FLASH_ACR_LATENCY_2;    
 
        RCC->CFGR  |= (u32)RCC_CFGR_HPRE_DIV1;                   // [7:4]   AHB  Ԥ��Ƶ, HCLK = SYSCLK/1  ����Ƶ     
        RCC->CFGR  |= (u32)RCC_CFGR_PPRE2_DIV1;                  // [13:11] APB2 Ԥ��Ƶ, APB2 = HCLK/1,   ����Ƶ
        RCC->CFGR  |= (u32)RCC_CFGR_PPRE1_DIV2;                  // [10: 8] APB1 Ԥ��Ƶ, APB1 = HCLK/2,    2��Ƶ

        RCC->CFGR  &= (u32)(~( 1<<16 | 0x01<<17 | 0xF<<18));     // ����
        RCC->CFGR  |= (u32)(0x01<<16 | 0x07<<18);                // PLL ʱ��Դ,��Ƶ���ӣ���Ƶϵ���� ʹPLLCK= HSE * 9= 72MHz   
        RCC->CR    |= (u32)(0x01<<24);                           // ʹ�� PLL    
        while((RCC->CR & RCC_CR_PLLRDY) == 0) {  }               // �ȴ�PLL����
    
        RCC->CFGR &= (u32)((u32)~(0x3<<0));                      // ��0
        RCC->CFGR |= (u32)(0x1 << 1);                            // �л�ϵͳʱ��ԴΪ��PLLCLOCK    
        while ((RCC->CFGR & (u32)RCC_CFGR_SWS) != (u32)0x08) { } // �ȴ�ϵͳʱ���л����
    }
    else
    { 
        // ��Ҫ������
        // ʱ�ӳ�ʼ��ʧ�� 
        // ����λ��
    }    
    // �ض����ж�������λ��
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
}



/*****************************************************************************
*��  ���� SystemCoreClockUpdate
*��  �ܣ� ����ʵ��ʱ��Ƶ�ʣ������ڣ�SystemCoreClock
*��  ����
*����ֵ��
*��  ע�� ע��_ħŮ������
*****************************************************************************/
void SystemCoreClockUpdate(void)
{
    u32 tmp = 0, pllmull = 0, pllsource = 0;    

    tmp = RCC->CFGR & RCC_CFGR_SWS;                                 // ��ȡʱ��Դ

    switch (tmp)  {
        case 0x00:                                                  // HSI �ڲ����پ��� ��ѡΪϵͳʱ��Դ
            SystemCoreClock = HSI_VALUE;
            break;
    
        case 0x04:                                                  // HSE �ⲿ���پ��� ��ѡΪϵͳʱ��Դ
            SystemCoreClock = HSE_VALUE;
            break;
    
        case 0x08:                                                  // PLL ����Ϊϵͳʱ��Դ     
            pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;                 // PLL ʱ��Դ����Ƶϵ��
            pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;       
            pllmull = ( pllmull >> 18) + 2;      
            if (pllsource == 0x00)
            {        
                SystemCoreClock = (HSI_VALUE >> 1) * pllmull;       // HSI����ʱ��2��Ƶ��ΪPLLʱ������
            }
            else
            {                                                       // HSE��ΪPLLʱ������                              
                if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) != (u32)RESET)
                {
                    SystemCoreClock = (HSE_VALUE >> 1) * pllmull;   // HSE 2��Ƶ
                }
                else
                {
                    SystemCoreClock = HSE_VALUE * pllmull;
                }
            }
            break;

        default:
            SystemCoreClock = HSI_VALUE;
            break;
    } 
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];  
    SystemCoreClock >>= tmp;    
}

