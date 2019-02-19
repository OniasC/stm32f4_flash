/*
******************************************************************************
File:     main.cpp
Info:     Generated by Atollic TrueSTUDIO(R) 9.0.1   2019-02-17

The MIT License (MIT)
Copyright (c) 2018 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "main.h"

/* Private macro */
/* Private variables */
/* Private function prototypes */
/* Private functions */

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/

uint32_t uwStartSector = 0;
uint32_t uwEndSector = 0;
uint32_t uwAddress = 0;
uint32_t uwSectorCounter = 0;

__IO uint32_t uwData32 = 0;
__IO uint32_t uwMemoryProgramStatus = 0;

static uint32_t GetSector(uint32_t Address);

void WriteFlash(uint32_t FLASH_SECTOR, uint32_t data);
int CheckData(uint32_t FLASH_SECTOR, uint32_t data);

int main(void)
{
  int i = 0;

  /**
  *  IMPORTANT NOTE!
  *  The symbol VECT_TAB_SRAM needs to be defined when building the project
  *  if code has been located to RAM and interrupts are used. 
  *  Otherwise the interrupt table located in flash will be used.
  *  See also the <system_*.c> file and how the SystemInit() function updates 
  *  SCB->VTOR register.  
  *  E.g.  SCB->VTOR = 0x20000000;  
  */

  /* Initialize LEDs */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);

  /* Initialize User Button*/
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

  /*We will only use one sector of flash memory - sector 3*/
  uint32_t FLASH_SECTOR;
  FLASH_SECTOR = ADDR_FLASH_SECTOR_3;

  uint32_t DATA_32;
  DATA_32 = 0x00000000;

  if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
  {
	  STM_EVAL_LEDOn(LED4);
	  STM_EVAL_LEDOn(LED6);
	  WriteFlash(FLASH_SECTOR, DATA_32);
  }

  //WriteFlash(FLASH_SECTOR, DATA_32);
  CheckData(FLASH_SECTOR, DATA_32);

  while (1)
  {
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	{
		DATA_32 += 0x00000001;
		if(DATA_32==0x00000005) DATA_32 = 0x00000001;

		WriteFlash(FLASH_SECTOR, DATA_32);
	}
	if(CheckData(FLASH_SECTOR, 0x00000001))
	{
		STM_EVAL_LEDOn(LED3);
		STM_EVAL_LEDOff(LED4);
		STM_EVAL_LEDOff(LED5);
		STM_EVAL_LEDOff(LED6);
	}
	else if(CheckData(FLASH_SECTOR, 0x00000002))
	{
		STM_EVAL_LEDOff(LED3);
		STM_EVAL_LEDOn(LED4);
		STM_EVAL_LEDOff(LED5);
		STM_EVAL_LEDOff(LED6);

	}
	else if(CheckData(FLASH_SECTOR, 0x00000003))
	{
		STM_EVAL_LEDOff(LED3);
		STM_EVAL_LEDOff(LED4);
		STM_EVAL_LEDOn(LED5);
		STM_EVAL_LEDOff(LED6);
	}
	else
	{
		STM_EVAL_LEDOff(LED3);
		STM_EVAL_LEDOff(LED4);
		STM_EVAL_LEDOff(LED5);
		STM_EVAL_LEDOn(LED6);
	}

    while(i<1000000)
    {
	    i++;
    }
    i = 0;
  }
}

void WriteFlash(uint32_t FLASH_SECTOR, uint32_t data)
{
  /* Enable the flash control register access */
  FLASH_Unlock();

  /* Erase the user Flash area ************************************************/
  /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */

  /* Clear pending flags (if any) */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

  uwStartSector = GetSector(FLASH_SECTOR);
  uwEndSector = GetSector(FLASH_SECTOR);

  /* Start the erase operation */
  uwSectorCounter = uwStartSector;
  while (uwSectorCounter <= uwEndSector)
  {
	/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	   be done by word */
	if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
	{
	  /* Error occurred while sector erase.
		 User can add here some code to deal with this error  */
	  while (1)
	  {
	  }
	}
	/* jump to the next sector */
	if (uwSectorCounter == FLASH_Sector_11)
	{
	  uwSectorCounter += 40;
	}
	else
	{
	  uwSectorCounter += 8;
	}
  }

  /* Program the user Flash area word by word ********************************/
  /* area defined by FLASH_SECTOR - only one sector being acted upon*/

  uwAddress = FLASH_SECTOR;

  while (uwAddress <= FLASH_SECTOR)
  {
	if (FLASH_ProgramWord(uwAddress, data) == FLASH_COMPLETE)
	{
	  uwAddress = uwAddress + 4;
	}
	else
	{
	  /* Error occurred while writing data in Flash memory.
		 User can add here some code to deal with this error */
	  while (1)
	  {
	  }
	}
  }

  /* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) */
  FLASH_Lock();
}

/* Check if the programmed data is OK ***************************************/
/*  MemoryProgramStatus = 0: data programmed correctly
    MemoryProgramStatus != 0: number of words not programmed correctly */
int CheckData(uint32_t FLASH_SECTOR, uint32_t data)
{
  uwAddress = FLASH_SECTOR;
  uwMemoryProgramStatus = 0;

  while (uwAddress <= FLASH_SECTOR)
  {
    uwData32 = *(__IO uint32_t*)uwAddress;

    if (uwData32 != data)
    {
      uwMemoryProgramStatus++;
    }

    uwAddress = uwAddress + 4;
  }

  /* Check Data correctness */
  if(uwMemoryProgramStatus)
  {
    /* KO */
    return 0;
    //STM_EVAL_LEDOn(LED5);
  }
  else
  {
    /* OK */
    //STM_EVAL_LEDOn(LED3);
    //STM_EVAL_LEDOn(LED5);
    return 1;
    //STM_EVAL_LEDOn(LED4);
    //STM_EVAL_LEDOn(LED6);
  }
}


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;
  }
  return sector;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
