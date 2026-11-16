/*********************************************************************************************************************************
*
********************* ESTE ARQUIVO É UMA VERSÃO CORRIGIDA E TRADUZIDA DO DRIVER ORIGINAL DO FABRICANTE ***************************
*
*********************************************************************************************************************************/

//************************************************************
// Comentário técnico: operação de transmissão/recepção do periférico USCI2
// Nome do arquivo: 		sc95f_usci2.h
// Autor:
// Função do módulo:			arquivo de cabeçalho da biblioteca de firmware USCI2
// Última revisão:	13/01/2023
// Versão:				V1.10003
// Observação:				Este arquivo é aplicável somente aos microcontroladores da série SC95F
//*************************************************************

#ifndef _sc95f_USCI2_H_
#define _sc95f_USCI2_H_

#include "sc95f.h"

#define USCI2_UART_BaudRate_FsysDIV12 0X00 // Disponível somente no modo 0; a porta serial opera a 1/12 do clock do sistema
#define USCI2_UART_BaudRate_FsysDIV4 0X01  // Disponível somente no modo 0; a porta serial opera a 1/4 do clock do sistema

typedef enum
{
  USCI2_Mode_SPI = (uint8_t)0x01, //SPI
  USCI2_Mode_TWI = (uint8_t)0x02, //TWI
  USCI2_Mode_UART = (uint8_t)0x03 //UART
} USCI2_CommunicationMode_TypeDef;

typedef enum
{
  USCI2_SPI_FIRSTBIT_MSB = (uint8_t)0x00, // Transmissão com MSB primeiro
  USCI2_SPI_FIRSTBIT_LSB = (uint8_t)0x04  // Transmissão com LSB primeiro
} USCI2_SPI_FirstBit_TypeDef;

typedef enum
{
  USCI2_SPI_BAUDRATEPRESCALER_1 = (uint8_t)0x00,  // Clock SPI = clock do sistema / 1
  USCI2_SPI_BAUDRATEPRESCALER_2 = (uint8_t)0x01,  // Clock SPI = clock do sistema / 2
  USCI2_SPI_BAUDRATEPRESCALER_4 = (uint8_t)0x02,  // Clock SPI = clock do sistema / 4
  USCI2_SPI_BAUDRATEPRESCALER_8 = (uint8_t)0x03,  // Clock SPI = clock do sistema / 8
  USCI2_SPI_BAUDRATEPRESCALER_16 = (uint8_t)0x04, // Clock SPI = clock do sistema / 16
  USCI2_SPI_BAUDRATEPRESCALER_32 = (uint8_t)0x05, // Clock SPI = clock do sistema / 32
  USCI2_SPI_BAUDRATEPRESCALER_64 = (uint8_t)0x06, // Clock SPI = clock do sistema / 64
  USCI2_SPI_BAUDRATEPRESCALER_128 = (uint8_t)0x07 // Clock SPI = clock do sistema / 128
} USCI2_SPI_BaudRatePrescaler_TypeDef;

typedef enum
{
  USCI2_SPI_MODE_MASTER = (uint8_t)0x20, // SPI configurado como mestre
  USCI2_SPI_MODE_SLAVE = (uint8_t)0x00   // SPI configurado como escravo
} USCI2_SPI_Mode_TypeDef;

typedef enum
{
  USCI2_SPI_CLOCKPOLARITY_LOW = (uint8_t)0x00, // SCK em nível baixo durante o estado ocioso
  USCI2_SPI_CLOCKPOLARITY_HIGH = (uint8_t)0x10 // SCK em nível alto durante o estado ocioso
} USCI2_SPI_ClockPolarity_TypeDef;

typedef enum
{
  USCI2_SPI_CLOCKPHASE_1EDGE = (uint8_t)0x00, // Amostra os dados na primeira borda de SCK
  USCI2_SPI_CLOCKPHASE_2EDGE = (uint8_t)0x08  // Amostra os dados na segunda borda de SCK
} USCI2_SPI_ClockPhase_TypeDef;

typedef enum
{
  USCI2_SPI_TXE_DISINT = (uint8_t)0x00, // Interrupção de transmissão desabilitada quando TBIE = 0
  USCI2_SPI_TXE_ENINT = (uint8_t)0x01   // Interrupção de transmissão habilitada quando TBIE = 1
} USCI2_SPI_TXE_INT_TypeDef;

typedef enum
{
  USCI2_SPI_DATA8 = (uint8_t)0x00, // Modo de transferência SPI de 8 bits
  USCI2_SPI_DATA16 = (uint8_t)0x01 // Modo de transferência SPI de 16 bits
} USCI2_TransmissionMode_TypeDef;

typedef enum
{
  USCI2_TWI_1024 = (uint8_t)0x00, // Taxa de comunicação TWI: Fhrc/1024
  USCI2_TWI_512 = (uint8_t)0x01,  // Taxa de comunicação TWI: Fhrc/512
  USCI2_TWI_256 = (uint8_t)0x02,  // Taxa de comunicação TWI: Fhrc/256
  USCI2_TWI_128 = (uint8_t)0x03,  // Taxa de comunicação TWI: Fhrc/128
  USCI2_TWI_64 = (uint8_t)0x04,   // Taxa de comunicação TWI: Fhrc/64
  USCI2_TWI_32 = (uint8_t)0x05,   // Taxa de comunicação TWI: Fhrc/32
  USCI2_TWI_16 = (uint8_t)0x06,   // Taxa de comunicação TWI: Fhrc/16
} USCI2_TWI_MasterCommunicationRate_TypeDef;

typedef enum
{
  USCI2_TWI_SlaveIdle = 0x00,
  USCI2_TWI_SlaveReceivedaAddress = 0x01,
  USCI2_TWI_SlaveReceivedaData = 0x02,
  USCI2_TWI_SlaveSendData = 0x03,
  USCI2_TWI_SlaveReceivedaUACK = 0x04,
  USCI2_TWI_SlaveDisableACK = 0x05,
  USCI2_TWI_SlaveAddressError = 0x06,
  USCI2_TWI_MasterIdle = 0x00,
  USCI2_TWI_MasterSendAddress = 0x01,
  USCI2_TWI_MasterSendData = 0x02,
  USCI2_TWI_MasterReceivedaData = 0x03,
  USCI2_TWI_MasterReceivedaUACK = 0x04,
} USCI2_TWIState_TypeDef;

typedef enum
{
  USCI2_UART_Mode_8B = 0X00,  // Configura UART no modo de 8 bits
  USCI2_UART_Mode_10B = 0X40, // Configura UART no modo de 10 bits
  USCI2_UART_Mode_11B = 0X80  // Configura UART no modo de 11 bits
} USCI2_UART_Mode_TypeDef;

typedef enum
{
  USCI2_UART_RX_ENABLE = 0X10, // Recepção UART habilitada
  USCI2_UART_RX_DISABLE = 0X00 // Recepção UART desabilitada
} USCI2_UART_RX_TypeDef;

typedef enum
{
  USCI2_SPI_FLAG_SPIF = (uint8_t)0x80, // Flag de transferência de dados SPI (SPIF)
  USCI2_SPI_FLAG_WCOL = (uint8_t)0x40, // Flag de colisão de escrita SPI (WCOL)
  USCI2_SPI_FLAG_TXE = (uint8_t)0x08,  // Flag de buffer de transmissão SPI vazio (TXE)
  USCI2_TWI_FLAG_TWIF = (uint8_t)0x40, // Flag de interrupção TWI (TWIF)
  USCI2_TWI_FLAG_GCA = (uint8_t)0x10,  // Flag de reconhecimento de endereço geral TWI (GCA)
  USCI2_TWI_FLAG_MSTR = (uint8_t)0x20, // Flag de seleção mestre/escravo TWI (MSTR)
  USCI2_TWI_FLAG_TXRXnE = (uint8_t)0x80,
  USCI2_UART_FLAG_RI = (uint8_t)0x01, // Flag de interrupção de recepção UART (RI)
  USCI2_UART_FLAG_TI = (uint8_t)0x02, // Flag de interrupção de transmissão UART (TI)
} USCI2_Flag_TypeDef;

typedef enum
{
  USCI2_TWI_Write = 0x00, // Operação de escrita
  USCI2_TWI_Read = 0x01,  // Operação de leitura
} USCI2_TWI_RWType;

typedef enum
{
  USCI2_STATE_READY = 0x00,   // Estado pronto
  USCI2_STATE_BUSY = 0x01,    // Estado ocupado
  USCI2_STATE_ERROR = 0x02,   // Estado de erro
  USCI2_STATE_TIMEOUT = 0x03,  // Estado de timeout
	USCI2_STATE_WAIT = 0x04,   // Estado de espera
} USCI2_StatusTypeDef;

typedef struct __USCI2_HandleInfoDef
{
  union
  {
    uint8_t* Size_u8;              // Buffer de transmissão de dados de 8 bits
    uint16_t* Size_u16;           // Buffer de transmissão de dados de 16 bits
  } pTxBuffPtr;
  uint8_t TxXferSize;              // Quantidade de dados a transmitir
  uint8_t TxXferCount;            // Contador de dados transmitidos
  union
  {
    uint8_t* Size_u8;              // Buffer de recepção de dados de 8 bits
    uint16_t* Size_u16;           // Buffer de recepção de dados de 16 bits
  } pRxBuffPtr;
  uint8_t RxXferSize;              // Quantidade de dados a receber
  uint8_t RxXferCount;              // Contador de dados recebidos
  volatile USCI2_StatusTypeDef TxState;      // Estado da transmissão
  USCI2_StatusTypeDef RxState;      // Estado da recepção
} USCI2_HandleInfoDef;

void USCI2_DeInit(void);
/********************************* SPI **************************************************/
void USCI2_SPI_Init(USCI2_SPI_FirstBit_TypeDef FirstBit,
                    USCI2_SPI_BaudRatePrescaler_TypeDef BaudRatePrescaler, USCI2_SPI_Mode_TypeDef Mode,
                    USCI2_SPI_ClockPolarity_TypeDef ClockPolarity, USCI2_SPI_ClockPhase_TypeDef ClockPhase,
                    USCI2_SPI_TXE_INT_TypeDef SPI_TXE_INT, USCI2_TransmissionMode_TypeDef TransmissionMode);
void USCI2_TransmissionMode(USCI2_TransmissionMode_TypeDef TransmissionMode);
void USCI2_SPI_Cmd(FunctionalState NewState);
void USCI2_SPI_SendData_8(uint8_t Data);
uint8_t USCI2_SPI_ReceiveData_8(void);
void USCI2_SPI_SendData_16(uint16_t Data);
uint16_t USCI2_SPI_ReceiveData_16(void);

StatusTypeDef USCI2_SPI_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_SPI_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_SPI_TransmitReceive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pTxData, uint8_t* pRxData, uint8_t Size, uint32_t Timeout);

StatusTypeDef USCI2_SPI_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);
StatusTypeDef USCI2_SPI_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_SPI_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_SPI_TransmitReceive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pTxData, uint8_t* pRxData, uint8_t Size);

/********************************* TWI **************************************************/
void USCI2_TWI_Slave_Init(uint8_t TWI_Address);
void USCI2_TWI_MasterCommunicationRate(USCI2_TWI_MasterCommunicationRate_TypeDef
                                       TWI_MasterCommunicationRate);
void USCI2_TWI_Start(void);
void USCI2_TWI_MasterModeStop(void);
void USCI2_TWI_SlaveClockExtension(FunctionalState NewState);
void USCI2_TWI_AcknowledgeConfig(FunctionalState NewState);
void USCI2_TWI_GeneralCallCmd(FunctionalState NewState);
FlagStatus USCI2_GetTWIStatus(USCI2_TWIState_TypeDef USCI2_TWIState);
void USCI2_TWI_Cmd(FunctionalState NewState);
void USCI2_TWI_SendAddr(uint8_t Addr, USCI2_TWI_RWType RW);
void USCI2_TWI_SendData(uint8_t Data);
uint8_t USCI2_TWI_ReceiveData(void);

StatusTypeDef USCI2_TWI_Master_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_TWI_Slave_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);

StatusTypeDef USCI2_TWI_Master_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_TWI_Master_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);

StatusTypeDef USCI2_TWI_Slave_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_TWI_Slave_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);

StatusTypeDef USCI2_TWI_Master_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_TWI_Slave_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);

StatusTypeDef USCI2_TWI_Master_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_TWI_Master_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);

StatusTypeDef USCI2_TWI_Slave_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_TWI_Slave_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);

/********************************* UART **************************************************/
void USCI2_UART_Init(uint32_t UART1Fsys, uint32_t BaudRate, USCI2_UART_Mode_TypeDef Mode,
                     USCI2_UART_RX_TypeDef RxMode);
void USCI2_UART_SendData8(uint8_t Data);
uint8_t USCI2_UART_ReceiveData8(void);
void USCI2_UART_SendData9(uint16_t Data);
uint16_t USCI2_UART_ReceiveData9(void);

StatusTypeDef USCI2_UART_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_UART_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_UART_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);
StatusTypeDef USCI2_UART_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout);
StatusTypeDef USCI2_UART_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size);
StatusTypeDef USCI2_UART_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE);

/*************** Interrupções e flags ***************/
void USCI2_ITConfig(FunctionalState NewState, PriorityStatus Priority);
FlagStatus USCI2_GetFlagStatus(USCI2_Flag_TypeDef USCI2_FLAG);
void USCI2_ClearFlag(USCI2_Flag_TypeDef USCI2_FLAG);

#endif

/******************* (C) COPYRIGHT 2022 SinOne Microelectronics *****END OF FILE****/