/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
*
* Program : Control  smi connected RTL8366
* Abstract :
* Author : Yu-Mei Pan (ympan@realtek.com.cn)
*  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
*/


#include "rtk_types.h"
#include "smi.h"
#include "rtk_error.h"
#include "platform.h"
#include "user_interface.h"
#include "osapi.h"


#define DELAY                        2
//#define CLK_DURATION(clk)            { os_delay_us(clk)}
#define _SMI_ACK_RESPONSE(ok)        { /*if (!(ok)) return RT_ERR_FAILED; */}

#define ack_timer                    5
#define max_register                0x018A

// GPIO IDs

static uint8_t smi_SDA;
static uint8_t smi_SCK;
//static uint8_t smi_RST;

void _smi_delay_ms(uint16_t ms)
{
  while (ms--)
  os_delay_us(1000);
}


void _smi_start(void)
{

  /* change GPIO pin to Output only */
  platform_gpio_mode(smi_SDA, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
  platform_gpio_mode(smi_SCK, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);

  /* Initial state: SCK: 0, SDA: 1 */
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  platform_gpio_write(smi_SDA, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);

  /* CLK 1: 0 -> 1, 1 -> 0 */
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  os_delay_us(DELAY);

  /* CLK 2: */
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SDA, PLATFORM_GPIO_LOW);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SDA, PLATFORM_GPIO_HIGH);

}



void _smi_writeBit(uint16 signal, uint32 bitLen)
{
  for( ; bitLen > 0; bitLen--)
  {
    os_delay_us(DELAY);

    /* prepare data */
    if ( signal & (1<<(bitLen-1)) )
		platform_gpio_write(smi_SDA, PLATFORM_GPIO_HIGH);
    else
		platform_gpio_write(smi_SDA, PLATFORM_GPIO_LOW);
    os_delay_us(DELAY);

    /* clocking */
    platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
    os_delay_us(DELAY);
    platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  }
}



void _smi_readBit(uint32 bitLen, uint32 *rData)
{
  uint32 u;

  /* change GPIO pin to Input only */
  //_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
  platform_gpio_mode(smi_SDA, PLATFORM_GPIO_INPUT, PLATFORM_GPIO_PULLUP);
  for (*rData = 0; bitLen > 0; bitLen--)
  {
    os_delay_us(DELAY);

    /* clocking */
    platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
    os_delay_us(DELAY);
    //_rtl865x_getGpioDataBit(smi_SDA, &u);
    u = platform_gpio_read(smi_SDA);
    platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);

    *rData |= (u << (bitLen - 1));
  }

  /* change GPIO pin to Output only */
  //_rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
  platform_gpio_mode(smi_SDA, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
}



void _smi_stop(void)
{

  os_delay_us(DELAY);
  platform_gpio_write(smi_SDA, PLATFORM_GPIO_LOW);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SDA, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);

  /* add a click */
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_LOW);
  os_delay_us(DELAY);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);


  /* change GPIO pin to Output only */
  platform_gpio_mode(smi_SDA, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
  platform_gpio_mode(smi_SCK, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);


}


int32 smi_reset(uint8 pinRST)
{
  int32 res;

  platform_gpio_mode(pinRST, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
  //if (res != PLATFORM_OK)
  //    return RT_ERR_FAILED;			todo: error handler

  platform_gpio_write(pinRST, PLATFORM_GPIO_HIGH);
  _smi_delay_ms(1000);
  platform_gpio_write(pinRST, PLATFORM_GPIO_LOW);
  _smi_delay_ms(1000);
  platform_gpio_write(pinRST, PLATFORM_GPIO_HIGH);
  _smi_delay_ms(1000);

  /* change GPIO pin to Input only */
  platform_gpio_mode(pinRST, PLATFORM_GPIO_INPUT, PLATFORM_GPIO_PULLUP);

  return RT_ERR_OK;
}


int32 smi_init(uint8 pinSCK, uint8 pinSDA)
{

  int32 res;

  smi_SDA = pinSDA;
  smi_SCK = pinSCK;
  //smi_RST = pinRST;


  /* change GPIO pin to Output only */
  platform_gpio_mode(smi_SDA, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
  platform_gpio_mode(smi_SCK, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);
  //platform_gpio_mode(smi_RST, PLATFORM_GPIO_OUTPUT, PLATFORM_GPIO_PULLUP);

  platform_gpio_write(smi_SDA, PLATFORM_GPIO_HIGH);
  platform_gpio_write(smi_SCK, PLATFORM_GPIO_HIGH);
  //platform_gpio_write(smi_RST, PLATFORM_GPIO_HIGH);
  return RT_ERR_OK;
}


int32 smi_read(uint32 mAddrs, uint32 *rData)
{

  uint32 rawData=0, ACK;
  uint8  con;
  uint32 ret = RT_ERR_OK;

  _smi_start();                                /* Start SMI */

  _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370 */

  _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

  _smi_writeBit(0x1, 1);                        /* 1: issue READ command */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for issuing READ command*/
  } while ((ACK != 0) && (con < ack_timer));

  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
  } while ((ACK != 0) && (con < ack_timer));

  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK by RTL8369 */
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_readBit(8, &rawData);                    /* Read DATA [7:0] */
  *rData = rawData&0xff;

  _smi_writeBit(0x00, 1);                        /* ACK by CPU */

  _smi_readBit(8, &rawData);                    /* Read DATA [15: 8] */

  _smi_writeBit(0x01, 1);                        /* ACK by CPU */
  *rData |= (rawData<<8);

  _smi_stop();

  return ret;
}



int32 smi_write(uint32 mAddrs, uint32 rData)
{

  int8 con;
  uint32 ACK;
  uint32 ret = RT_ERR_OK;

  _smi_start();                                /* Start SMI */

  _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370*/

  _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

  _smi_writeBit(0x0, 1);                        /* 0: issue WRITE command */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for issuing WRITE command*/
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[15:8] */
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit(rData&0xff, 8);                /* Write Data [7:0] out */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                    /* ACK for writting data [7:0] */
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_writeBit(rData>>8, 8);                    /* Write Data [15:8] out */

  con = 0;
  do {
    con++;
    _smi_readBit(1, &ACK);                        /* ACK for writting data [15:8] */
  } while ((ACK != 0) && (con < ack_timer));
  if (ACK != 0)
    ret = RT_ERR_FAILED;

  _smi_stop();


  return ret;
}
