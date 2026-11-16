/*
 * TKDriver.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Sinone
 */

#ifndef	_TKDriver_H
#define	_TKDriver_H

// Includes --------------------------------------------------------------------
// Exported types --------------------------------------------------------------
typedef	enum
{
	Circle = 0,		
	Bar = 1,			
    wheel = 0,
    slider = 1
}TKSlideModuleType;

typedef struct
{
	TKSlideModuleType TypeFlag;         // Tipo de interface touch (slider, wheel, bar, circle)
	unsigned char TKChannel[16];        // Canais touch em ordem de ativação
	unsigned char UsingTKChannelNumber; // Número de canais touch
	unsigned int SideLevel;	            // Número máximo de níveis/posições do slider
	int SUBData;                        // Se o valor diferencial for menor que o valor ajustado para esta variável, a saída será zero.
	int LIBData;                        // Se o valor diferencial for maior que o valor ajustado para esta variável, considera-se tecla ativa. 
	unsigned char TKOrderChannel[16];   // Armazena os índices da ordenação interna dos canais TK.
	unsigned char MAXUpdateCount;       // Número de ciclos de varredura sem detectar uma tecla do slider antes de atualizar a base line.
	unsigned char LastOutValue;         // Registra o último valor de saída.
	unsigned int OutValue;              // Valor de saída do slider.
	unsigned int UpdateBaseLineNumber;  // Contador de atualizações da base line.
	unsigned int CouplingValue;         // Faixa do parâmetro de acoplamento (100~200). Se for muito pequeno, podem faltar valores na saída, por exemplo: 1 -> 2 -> 3 -> 5 -> 6 -> 7 Se for muito grande, podem ocorrer valores repetidos/sobrepostos: 1 -> 2 -> 3 -> 4 -> 3 -> 4 -> 5 -> 6 -> 7
	unsigned int DebugCouplingValue;    // Valor de acoplamento para debug. Útil para ajustar o CouplingValue (usar menor valor obtido).
	unsigned int TriggerFlagCount;      // Contador de acionamentos do slider.
	
}TKSlideModulePCB;

// Exported constants ----------------------------------------------------------
#define USING_TKSlideModule_Number_Set          1           //USING_TKSlideModule_Number_Set 
#define TypeFlag0                               slider      //TypeFlag 
#define UsingTKChannelNumber0                   6           //UsingTKChannelNumber 
#define SideLevel0                              11          //SideLevel 
#define TKChannel0                              0,1,2,3,4,5 //TKChannel 
#define LIBData0                                240         //LIBData 

// Exported enumerations -------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
// Exported variables ----------------------------------------------------------
extern unsigned char xdata SOCAPI_TouchKeyStatus;
extern TKSlideModulePCB   TKSlideModulePCBArray[];
extern unsigned long int TouchKeyScan();
extern void TouchKeyInit();
extern void TouchKeyRestart();

#endif 
