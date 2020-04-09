  v/****************************************Copyright (c)****************************************************
**
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               OV7670.c
** Descriptions:            OV7670 application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2011-12-12
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SCCB.h"
#include "OV7670.h"
#include "systick.h"

/* Private variables ---------------------------------------------------------*/

uint8_t Vsync;         /* 帧同步信号 */

const uint8_t OV7670_Reg[OV7670_REG_NUM][2]=
{
     /*  OV7670 QVGA RGB565参数  */
        {0x3a, 0x04},//
        {0x40, 0x10},
        {0x12, 0x14},
        {0x32, 0x80},
        {0x17, 0x16},

        {0x18, 0x04},
        {0x19, 0x02},
        {0x1a, 0x7b},
        {0x03, 0x06},
        {0x0c, 0x0c},
        {0x15, 0x02},
        {0x3e, 0x00},
        {0x70, 0x00},
        {0x71, 0x01},
        {0x72, 0x11},
        {0x73, 0x09},

        {0xa2, 0x02},
        {0x11, 0x00},
        {0x7a, 0x20},
        {0x7b, 0x1c},
        {0x7c, 0x28},

        {0x7d, 0x3c},
        {0x7e, 0x55},
        {0x7f, 0x68},
        {0x80, 0x76},
        {0x81, 0x80},

        {0x82, 0x88},
        {0x83, 0x8f},
        {0x84, 0x96},
        {0x85, 0xa3},
        {0x86, 0xaf},

        {0x87, 0xc4},
        {0x88, 0xd7},
        {0x89, 0xe8},
        {0x13, 0xe0},
        {0x00, 0x00}, /* AGC */

        {0x10, 0x00},
        {0x0d, 0x00},
        {0x14, 0x20},
        {0xa5, 0x05},
        {0xab, 0x07},

        {0x24, 0x75},
        {0x25, 0x63},
        {0x26, 0xA5},
        {0x9f, 0x78},
        {0xa0, 0x68},

        {0xa1, 0x03},
        {0xa6, 0xdf},
        {0xa7, 0xdf},
        {0xa8, 0xf0},
        {0xa9, 0x90},

        {0xaa, 0x94},
        {0x13, 0xe5},
        {0x0e, 0x61},
        {0x0f, 0x4b},
        {0x16, 0x02},

        {0x1e, 0x37},
        {0x21, 0x02},
        {0x22, 0x91},
        {0x29, 0x07},
        {0x33, 0x0b},

        {0x35, 0x0b},
        {0x37, 0x1d},
        {0x38, 0x71},
        {0x39, 0x2a},
        {0x3c, 0x78},

        {0x4d, 0x40},
        {0x4e, 0x20},
        {0x69, 0x5d},
        {0x6b, 0x40},
        {0x74, 0x19},
        {0x8d, 0x4f},

        {0x8e, 0x00},
        {0x8f, 0x00},
        {0x90, 0x00},
        {0x91, 0x00},
        {0x92, 0x00},

        {0x96, 0x00},
        {0x9a, 0x80},
        {0xb0, 0x84},
        {0xb1, 0x0c},
        {0xb2, 0x0e},

        {0xb3, 0x82},
        {0xb8, 0x0a},
        {0x43, 0x14},
        {0x44, 0xf0},
        {0x45, 0x34},

        {0x46, 0x58},
        {0x47, 0x28},
        {0x48, 0x3a},
        {0x59, 0x88},
        {0x5a, 0x88},

        {0x5b, 0x44},
        {0x5c, 0x67},
        {0x5d, 0x49},
        {0x5e, 0x0e},
        {0x64, 0x04},
        {0x65, 0x20},

        {0x66, 0x05},
        {0x94, 0x04},
        {0x95, 0x08},
        {0x6c, 0x0a},
        {0x6d, 0x55},

        {0x4f, 0x80},
        {0x50, 0x80},
        {0x51, 0x00},
        {0x52, 0x22},
        {0x53, 0x5e},
        {0x54, 0x80},

        {0x6e, 0x11},
        {0x6f, 0x9f},
    {0x55, 0x00}, /* 亮度 */
    {0x56, 0x40}, /* 对比度 */
    {0x57, 0x80}, /* change according to Jim's request */
};

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : OV7725 GPIO Configuration
* Input          : None
* Output         : None
* Return         : None
* Attention                 : None
*******************************************************************************/
static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_PORT_VSYNC_CMOS, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  PIN_VSYNC_CMOS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(PORT_VSYNC_CMOS, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : FIFO_GPIO_Configuration
* Description    : FIFO GPIO Configuration
* Input          : None
* Output         : None
* Return         : None
* Attention                 : None
*******************************************************************************/
void FIFO_GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);

  /* FIFO_RCLK : PE1 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* FIFO_RRST : PE0 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* FIFO_CS : PD6 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* FIFO_WEN : PD3 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* FIFO D[0-7] */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* FIFO_HREF : PB7 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : OV7670_NVIC_Configuration
* Description    : VSYNC中断配置
* Input          : None
* Output         : None
* Return         : None
* Attention                 : None
*******************************************************************************/
void OV7670_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : OV7670_EXTI_Configuration
* Description    : VSYNC中断管脚配置
* Input          : None
* Output         : None
* Return         : None
* Attention                 : None
*******************************************************************************/
void OV7670_EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

  GPIO_EXTILineConfig(PORT_SOURCE_VSYNC_CMOS, PIN_SOURCE_VSYNC_CMOS);
  EXTI_InitStructure.EXTI_Line = EXTI_LINE_VSYNC_CMOS;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_GenerateSWInterrupt(EXTI_LINE_VSYNC_CMOS);
}

/*******************************************************************************
* Function Name  : Sensor_Init
* Description    : Sensor初始化
* Input          : None
* Output         : None
* Return         : None
* Attention                 : 返回1成功，返回0失败
*******************************************************************************/
int Sensor_Init(void)
{
  uint16_t i=0;
  uint8_t Sensor_IDCode = 0;

  delay_init();

  GPIO_Configuration();

  I2C_Configuration();

  if( 0 == I2C_WriteByte ( 0x12, 0x80 , ADDR_OV7670 ) ) /* Reset SCCB */
  {
     return 0 ;
  }

  delay_ms(50);

  if( 0 == I2C_ReadByte( &Sensor_IDCode, 1, 0x0b,  ADDR_OV7670 ) )         /* read ID */
  {
         return 0;                                      /* error*/
  }
  if(Sensor_IDCode == OV7670)                                  /* ID = OV7670 */
  {
           for( i=0 ; i < OV7670_REG_NUM ; i++ )
           {
                 if( 0 == I2C_WriteByte(  OV7670_Reg[i][0], OV7670_Reg[i][1] , ADDR_OV7670 ) )
            {
                        return 0;
                 }
         }
  }
  else                                                                                  /* NO ID */
  {
      return 0;
  }

  return 1;
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/