#include "lowpower.h"



void Enter_Stop_Mode(void)
{
	/* Enable Wakeup Counter */
	RTC_WakeUpCmd(ENABLE);
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
	RTC_WakeUpCmd(DISABLE);	
}

