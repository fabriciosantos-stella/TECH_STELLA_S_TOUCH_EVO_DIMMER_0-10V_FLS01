/*
 * Slider.c
 *
 *  Created on: Jun 10, 2026
 *      Author: Sinone
 */

// Includes --------------------------------------------------------------------
#include "S_TouchKeyCFG.h"
#include "TKDriver.h"

// Private define --------------------------------------------------------------
#define USING_SlideModule   // Define uso de sensor slider

// Configurações básicas da biblioteca
#define SOCAPI_CFG_CYCLE_CNT            32 		// Faixa: 10~255. Número de ciclos usados para filtrar os dados durante a inicialização e para inicializar os valores de correção. Quanto maior o valor, mais longa será a inicialização, porém maior será a estabilidade dos dados do sensor.
#define	SOCAPI_CFG_OVERLOW_MAX_COUNT    10		// Configuração da velocidade de atualização em condição de baseline baixa. Define após quantos ciclos a baseline será atualizada. Incremento de atualização: noise/2. Quanto menor o valor, mais rápida a atualização da baseline.
#define	SOCAPI_CFG_RESET_BASELINE_CNT	10  	// Após quantas atualizações em condição de baseline baixa a baseline será forçada a ser recalculada. Quanto menor o valor, mais rápida a atualização. O produto: SOCAPI_CFG_OVERLOW_MAX_COUNT * SOCAPI_CFG_RESET_BASELINE_CNT, não pode ser maior que 255.

#define USING_TKSlideModule_Number USING_TKSlideModule_Number_Set

#define CONFIRMTOUCHCNT        	        3
#define INIT_AUTO_UPDATE_TIME	 	    4
#define SET_KEY_CONTI_TIME     	        5  
#define SET_UPDATE_SPEED 		 		7	
#define AUTO_UPDATE_TIME	     	    8
#define	SET_ANTIJAM		     			10
#define NOISE                  	        16
#define SET_TOUCH_FREQ		 			0
#define	SET_RESOLUTION		 			1
#define SET_GAIN_CFG			 		2
#define SCANTIME				 		3
#define SET_ICHA				 		4
#define FINGER_THRESHOLD_H	 		    6
#define FINGER_THRESHOLD_L	 		    7

// Private typedef -------------------------------------------------------------
// Private enumerations --------------------------------------------------------
// Private variables -----------------------------------------------------------
//*************************************************************************************
// Definições dos registradores 844 (registradores especificos do microcontrolador)
//*************************************************************************************
sfr TK_PSW = 0xD0;	    // Palavra de Status do Programa (Program Status Word)
sbit TK_CY = TK_PSW^7;	// Bit de transporte (Carry)

unsigned char xdata	SOCAPI_TouchKeyStatus;  // Estado da API:
                                            // bit7 - flag de conclusão de uma rodada de varredura
                                            //        1 = concluída
                                            //        0 = não concluída
                                            //
                                            // bit6 - flag de estouro do contador de toque do canal
                                            //        1 = overflow
                                            //        0 = normal
                                                        
unsigned int xdata RawData [SOCAPI_SET_TOUCHKEY_TOTAL];			
unsigned int xdata BaseLine[SOCAPI_SET_TOUCHKEY_TOTAL];
unsigned int xdata FilterData[SOCAPI_SET_TOUCHKEY_TOTAL];												
unsigned char xdata RestAreaCnt[SOCAPI_SET_TOUCHKEY_TOTAL]; 			
unsigned char xdata	TouchCnt[SOCAPI_SET_TOUCHKEY_TOTAL];				
unsigned char xdata	NoTouchCnt[SOCAPI_SET_TOUCHKEY_TOTAL];  			
unsigned char xdata	CurrentChannel[SOCAPI_SET_TOUCHKEY_TOTAL];						     
unsigned char xdata	FloatAreaCnt[SOCAPI_SET_TOUCHKEY_TOTAL];  	
unsigned char xdata BaseLineAdjusttmp[SOCAPI_SET_TOUCHKEY_TOTAL];
int xdata DifferAccum[SOCAPI_SET_TOUCHKEY_TOTAL]; 
char xdata SetNoiseThreshold;
unsigned char xdata ConfirmTouchCnt;
TKSlideModulePCB TKSlideModulePCBArray[USING_TKSlideModule_Number] = 
{
    {
        TypeFlag0,			    // TypeFlag
        {TKChannel0},	        // TKChannel
        UsingTKChannelNumber0,	// UsingTKChannelNumber
        SideLevel0,			    // SideLevel
        20,			            // SUBData
        LIBData0,		        // LIBData
        {0},		            // TKOrderChannel
        1,			            // MAXUpdateCount
        0,			            // LastOutValue
        0,			            // OutValue
        0,			            // UpdateBaseLineNumber
        150,		            // CouplingValue
        0,			            // DebugCouplingValue
        0			            // TriggerFlagCount
    }
}; 

unsigned char code CFG_OVERLOW_MAX_COUNT = SOCAPI_CFG_OVERLOW_MAX_COUNT;
unsigned char code CFG_RESET_BASELINE_CNT = SOCAPI_CFG_RESET_BASELINE_CNT;	
unsigned char code CFG_CYCLE_CNT = SOCAPI_CFG_CYCLE_CNT;

// Exported variables ----------------------------------------------------------
extern unsigned	char data CurrentChannelMax;
extern unsigned char data UsingTKSlideModuleNumber;

// Private function prototypes -------------------------------------------------
// Exported function prototypes ------------------------------------------------
extern bit  GetIsNeedUpdateBaseline(void);
extern void SetNeedUpdateBaseline(void);
extern unsigned long int SensorKeyFlag(void);
extern void MultipleDeal(unsigned char CycleCnt);
extern void TouchKey_Service(void);

// Private functions -----------------------------------------------------------
// *****************************************************************************
/// @brief	Define o tempo máximo de saída/acionamento contínuo de uma tecla
/// @fn		unsigned int SetOneKeyPushResetTime(void)
/// @Param	None            @brief  None
/// @return None            @brief  None
// *****************************************************************************
unsigned int SetOneKeyPushResetTime(void)   
{	  
	return  TKCFG[SET_KEY_CONTI_TIME];
}

// *****************************************************************************
/// @brief	Velocidade de atualização da linha de base (baseline)
/// @fn		int GetBaselineUpdateThreshold(void)
/// @Param	None            @brief  None
/// @return int             @brief  Velocidade de atualização
// *****************************************************************************
int GetBaselineUpdateThreshold(void)
{
	return TKCFG[SET_UPDATE_SPEED]; 
}

// *****************************************************************************
/// @brief	Número de ciclos de autocalibração durante a inicialização
/// @fn		unsigned char GetInitAutoUpdateTime(void)
/// @Param	None                @brief  None
/// @return unsigned char       @brief  Número de ciclos de autocalibração
// *****************************************************************************
unsigned char GetInitAutoUpdateTime(void)
{
	return  TKCFG[INIT_AUTO_UPDATE_TIME];
}

// *****************************************************************************
/// @brief	Obtém o valor atual do "finger" (toque detectado)
/// @fn		unsigned int GetCurrFingerValue(unsigned char i)
/// @Param	unsigned char      @brief  Indice do canal
/// @return unsigned int       @brief  Valor do canal
// *****************************************************************************
unsigned int GetCurrFingerValue(unsigned char i)
{ 
	return	TKChannelCfg[i][FINGER_THRESHOLD_H]*256+TKChannelCfg[i][FINGER_THRESHOLD_L] ;
}

// *****************************************************************************
/// @brief	Obtém o tempo de varredura (scan) do canal atual
/// @fn		unsigned int GetCurrFingerValue(unsigned char i)
/// @Param	unsigned char      @brief  Indice do canal
/// @return unsigned char      @brief  Tempo de varredura
// *****************************************************************************
unsigned char GetScanTimeValue(unsigned char i)
{ 
	return TKChannelCfg[i][SCANTIME];
}

// *****************************************************************************
/// @brief	Obtém o valor de ajuste da linha de base (baseline) do canal atual
/// @fn		unsigned char GetBaseLineAdjustValue(unsigned char i)
/// @Param	unsigned char      @brief  Indice do canal
/// @return unsigned char      @brief  Valor do base line ajustado
// *****************************************************************************
unsigned char GetBaseLineAdjustValue(unsigned char i)
{	
	return BaseLineAdjusttmp[i]; 
}

// *****************************************************************************
/// @brief	Obtém a quantidade de confirmações para detecção da liberação da tecla
/// @fn		unsigned char GetUpConfirmCnt(void)
/// @Param	none               @brief  none
/// @return unsigned char      @brief  Valor do contador
// *****************************************************************************
unsigned char GetUpConfirmCnt(void)
{
	return ConfirmTouchCnt>>1;
}

// *****************************************************************************
/// @brief	Seleção do sensor/canal atual
/// @fn		void CurrentSensorChoose(void)
/// @Param	none      @brief  none
/// @return none      @brief  none
// *****************************************************************************
void CurrentSensorChoose(void)
{
	unsigned char i = 0;
	unsigned char Ctk_Channel_mark = 0;
	unsigned long int CurrentSensorKey = 0 ; 
	unsigned char k, j = 0;
	
	CurrentSensorKey = SOCAPI_SET_TOUCHKEY_CHANNEL; 
		
	for(i = 0; i < 31; i++)
	{
		CurrentSensorKey=CurrentSensorKey>>1;
		if(TK_CY)
		{
			CurrentChannel[Ctk_Channel_mark] = i;
			Ctk_Channel_mark++;
			if(Ctk_Channel_mark >= SOCAPI_SET_TOUCHKEY_TOTAL)
				break;
		}		
	}
	
	for(k = 0; k < UsingTKSlideModuleNumber; k ++)
	{
		for(i = 0;i<TKSlideModulePCBArray[k].UsingTKChannelNumber;i++)
		{
			for(j=0;j<Ctk_Channel_mark;j++)
			{
				if(TKSlideModulePCBArray[k].TKChannel[i] == CurrentChannel[j])
				{
					TKSlideModulePCBArray[k].TKOrderChannel[i]=j;	
				}
			}	
		}
	}
	CurrentChannelMax = Ctk_Channel_mark;
}

// *****************************************************************************
/// @brief	Obtém as informações de configuração do Touch Key
/// @fn		unsigned char GetCfgMsg(unsigned char i)
/// @Param	unsigned char      @brief  Indice do parametro desejado
/// @return unsigned char      @brief  Valor configurado
// *****************************************************************************
unsigned char GetCfgMsg(unsigned char i)
{
	switch(i)
	{ 
		 case 0:  return TKChannelCfg[0][SET_TOUCH_FREQ];
		 case 1:  return TKChannelCfg[0][SET_RESOLUTION];
		 case 2:  return TKChannelCfg[0][SET_GAIN_CFG];
		 case 3:  return GetBaseLineAdjustValue(0);
		 case 4:  return TKCFG[SET_ANTIJAM];
		 default:return 0; 	 	
	}
}

// *****************************************************************************
/// @brief	Inicializa os registradores TK
/// @fn		void TouchKeyCFGInit(void)
/// @Param	none      @brief  none
/// @return none      @brief  none
// *****************************************************************************
void TouchKeyCFGInit(void)
{
	unsigned char  	i;
	unsigned char  data	k;
	
	UsingTKSlideModuleNumber = USING_TKSlideModule_Number;
	ConfirmTouchCnt = TKCFG[CONFIRMTOUCHCNT];
	SetNoiseThreshold = TKCFG[NOISE];
	CurrentSensorChoose(); 
	for(i=0;i<CurrentChannelMax;i++)
	{
	  BaseLineAdjusttmp[i] =TKChannelCfg[i][SET_ICHA];;	
	} 

	for(k = 0; k < UsingTKSlideModuleNumber; k ++)
	{
		TKSlideModulePCBArray[k].UpdateBaseLineNumber = 0;
	}
}

// *****************************************************************************
/// @brief	Interface de detecção de teclas touch
/// @fn		unsigned long int TouchKeyScan(void)
/// @Param	none      @brief  none
/// @return none      @brief  none
// *****************************************************************************
unsigned long int TouchKeyScan(void)
{
	unsigned char k;
	unsigned long int Keyvalue; 

    if(GetIsNeedUpdateBaseline() == 0)
	{
		Keyvalue = SensorKeyFlag();

		for(k = 0; k < UsingTKSlideModuleNumber; k ++)
		{
			if(TKSlideModulePCBArray[k].OutValue != 0x00)
			{	
				if(TKSlideModulePCBArray[k].OutValue == TKSlideModulePCBArray[k].LastOutValue)
				{		
				   TKSlideModulePCBArray[k].UpdateBaseLineNumber++; 
				}
				else
				{
				   TKSlideModulePCBArray[k].LastOutValue = TKSlideModulePCBArray[k].OutValue;
				   TKSlideModulePCBArray[k].UpdateBaseLineNumber = 0; 
				}
			}
			else	
			{
				TKSlideModulePCBArray[k].UpdateBaseLineNumber = 0; 	
			}

			if(TKSlideModulePCBArray[k].UpdateBaseLineNumber >= SetOneKeyPushResetTime())
			{
				SetNeedUpdateBaseline(); 
				TKSlideModulePCBArray[k].UpdateBaseLineNumber = 0;
			}
		}
				
	}			
	else
	{
		MultipleDeal(TKCFG[AUTO_UPDATE_TIME]);
	}  
	
	return 0;
}

// *****************************************************************************
/// @brief	Função de tratamento a interrupção TK
/// @fn		void CTK_ISR(void) interrupt	11
/// @Param	none      @brief  none
/// @return none      @brief  none
// *****************************************************************************
void CTK_ISR(void) interrupt	11
{
   TouchKey_Service();
}



