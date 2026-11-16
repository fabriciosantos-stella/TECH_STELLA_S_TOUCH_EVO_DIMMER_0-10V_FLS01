/*********************************************************************************************************************************
*
********************* ESTE ARQUIVO É UMA VERSÃO CORRIGIDA E TRADUZIDA DO DRIVER ORIGINAL DO FABRICANTE ***************************
*
*********************************************************************************************************************************/

//************************************************************
// Comentário técnico: operação de transmissão/recepção do periférico USCI2
// Nome do arquivo: 		sc95f_usci2_stella.c
// Autor		:
// Função do módulo: 		arquivo C da biblioteca de firmware USCI2
// Última revisão: 16/02/2023
// Versão: 				V1.10011
// Observação:Este arquivo é aplicável somente aos microcontroladores da série SC95F
//*************************************************************

#include "sc95f_usci2.h"

#if defined(SC95F8x3x) || defined(SC95F7x3x) || defined (SC95F8x6x) || defined (SC95F7x6x)  || defined (SC95F8x1xB) || defined (SC95F7x1xB)\
 || defined (SC95R751) || defined (SC95F7610B) || defined (SC95F7619B) || defined (SC95R602)  || defined (SC95R605)
#define Select_USCI2()          \
  do {                       \
    USXINX &= 0xF8;          \
    USXINX |= 0X02;          \
  } while(0)

#define USCI2_SetNull() TMCON = (TMCON & 0X3F)
#define USCI2_SetSPI()	TMCON = (TMCON & 0X3F) | 0X40
#define USCI2_SetTWI()	TMCON = (TMCON & 0X3F) | 0X80
#define USCI2_SetUART()	TMCON = (TMCON & 0X3F) | 0XC0

#define US2CON0 USXCON0
#define US2CON1 USXCON1
#define US2CON2 USXCON2
#define US2CON3 USXCON3

#else

#define Select_USCI2()

#define USCI2_SetNull() TMCON = (TMCON & 0X3F)
#define USCI2_SetSPI() TMCON = (TMCON & 0X3F) | 0X40
#define USCI2_SetTWI() TMCON = (TMCON & 0X3F) | 0X80
#define USCI2_SetUART() TMCON = (TMCON & 0X3F) | 0XC0

#endif

/**
 * Função: USCI2_DeInit
 * Descrição: Inicializa e configura o periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_DeInit(void)
{
  Select_USCI2();
  USCI2_SetNull();
  US2CON0 = 0X00;
  US2CON1 = 0X00;
  US2CON2 = 0X00;
  US2CON3 = 0X00;
  IE2 &= (~0X02);
  IP2 &= (~0X02);
}


/*************** Funções SPI ***************/
/**
 * Função: USCI2_SPI_Init
 * Descrição: Inicializa e configura o periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_SPI_Init(USCI2_SPI_FirstBit_TypeDef FirstBit,
                    USCI2_SPI_BaudRatePrescaler_TypeDef BaudRatePrescaler, USCI2_SPI_Mode_TypeDef Mode,
                    USCI2_SPI_ClockPolarity_TypeDef ClockPolarity, USCI2_SPI_ClockPhase_TypeDef ClockPhase,
                    USCI2_SPI_TXE_INT_TypeDef SPI_TXE_INT, USCI2_TransmissionMode_TypeDef TransmissionMode)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetSPI();// Configura o USCI2 no modo SPI
  SPI_TXE_INT = USCI2_SPI_TXE_DISINT; // A função SPI_TXE_INT não está disponível neste dispositivo USCI2
  US2CON1 = US2CON1 & (~0X05) | FirstBit | TransmissionMode;
  US2CON0 = US2CON0 & 0X80 | BaudRatePrescaler | Mode | ClockPolarity | ClockPhase;
}

/**
 * Função: USCI2_TransmissionMode
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TransmissionMode(USCI2_TransmissionMode_TypeDef TransmissionMode)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetSPI();// Configura o USCI2 no modo SPI
  if(TransmissionMode == USCI2_SPI_DATA8)
  {
    US2CON1 &= 0xFD;
  }
  else
  {
    US2CON1 |= 0x02;
  }
}

/**
 * Função: USCI2_SPI_Cmd
 * Descrição: Habilita ou desabilita a função do periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_SPI_Cmd(FunctionalState NewState)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetSPI();// Configura o USCI2 no modo SPI

  if(NewState != DISABLE)
  {
    US2CON0 |= 0X80;
  }
  else
  {
    US2CON0 &= (~0X80);
  }
}

/**
 * Função: USCI2_SPI_SendData_8
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_SPI_SendData_8(uint8_t Data)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  US2CON2 = Data;
}

/**
 * Função: USCI2_SPI_ReceiveData_8
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
uint8_t USCI2_SPI_ReceiveData_8(void)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  return US2CON2;
}

/**
 * Função: USCI2_SPI_SendData_16
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_SPI_SendData_16(uint16_t Data)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  US2CON3 = (uint8_t)(Data >> 8);
  US2CON2 = (uint8_t)Data;
}

/**
 * Função: USCI2_SPI_ReceiveData_16
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
uint16_t USCI2_SPI_ReceiveData_16(void)
{
  uint16_t SPI_data;
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  SPI_data = (uint16_t)((US2CON3 << 8) | US2CON2);
  return SPI_data;
}

/**
 * Função: USCI2_SPI_IRQHandler
 * Descrição: Processa o evento do periférico no contexto da interrupção.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  StatusTypeDef TempStatus = Status_ERROR;

  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  if(USCI2_HANDLE->RxState == USCI2_STATE_BUSY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if((US2CON1 & USCI2_SPI_DATA16) != 0)
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u16 + USCI2_HANDLE->RxXferCount) = USCI2_SPI_ReceiveData_16();			// Lê 16 bits de dados
      }
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      else
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = USCI2_SPI_ReceiveData_8();			// Lê 16 bits de dados
      }

      TempStatus = Status_BUSY;
      USCI2_HANDLE->RxXferCount++;			// Dado recebido; incrementa o contador

      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
      {
        USCI2_HANDLE->RxState = USCI2_STATE_READY;
        TempStatus =  Status_OK;
      }
    }

    if(USCI2_HANDLE->TxState == USCI2_STATE_BUSY)
    {

      USCI2_HANDLE->TxXferCount++;			// Transmissão de um dado concluída; incrementa o contador
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        if((US2CON1 & USCI2_SPI_DATA16) != 0)
        {
          USCI2_SPI_SendData_16(*(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount));			// Transmite dados de 16 bits
        }
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        else
        {
          USCI2_SPI_SendData_8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount));			// Transmite dados de 16 bits
        }
        TempStatus = Status_BUSY;
      }
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      else if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
      {
        USCI2_HANDLE->TxState = USCI2_STATE_READY;
        TempStatus = Status_OK;
      }
    }
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    else if(USCI2_HANDLE->RxXferSize)
    {
      USCI2_SPI_SendData_8(0x00);
    }
  }
  return TempStatus;
}

/**
 * Função: USCI2_SPI_Receive
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  uint32_t TimeoutCnt = 0;

  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return USCI2_STATE_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((US2CON0 & USCI2_SPI_MODE_MASTER) != 0)
    {
      return USCI2_SPI_TransmitReceive(USCI2_HANDLE, pData, pData, Size, Timeout);			// Encaminha para a rotina de transferência bloqueante
    }

    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			 // Limpa a flag de recepção antes de receber
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    while(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if(USCI2_GetFlagStatus(USCI2_SPI_FLAG_SPIF))
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        if((US2CON1 & USCI2_SPI_DATA16) != 0)
        {
          *(USCI2_HANDLE->pRxBuffPtr.Size_u16 + USCI2_HANDLE->RxXferCount) = USCI2_SPI_ReceiveData_16();			// Lê 16 bits e incrementa o endereço do buffer
        }
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        else
        {
          *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = USCI2_SPI_ReceiveData_8();			// Lê 16 bits e incrementa o endereço do buffer
        }
        USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);				// Limpa a flag
        USCI2_HANDLE->RxXferCount++;	// Incrementa o contador de dados recebidos
        TimeoutCnt = 0;			// Zera o contador de timeout
      }
      else
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        if((TimeoutCnt++) > Timeout)
        {
          if(Timeout == 0)
            return USCI2_STATE_TIMEOUT;				// Retorna erro de timeout
        }
        WDTCON |= 0x10;          // Alimenta o watchdog para evitar reset
      }
    }
    if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
    {
      USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Recepção concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;// Erro de recepção
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;// Retorna o estado ocupado
  }
}

/**
 * Função: USCI2_SPI_Receive_IT
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 == 0) || (Size == 0U))
    {
      return USCI2_STATE_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((US2CON0 & USCI2_SPI_MODE_MASTER) != 0)
    {
      return USCI2_SPI_TransmitReceive_IT(USCI2_HANDLE, pData, pData, Size);			// Encaminha para a rotina de transferência por interrupção
    }

    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Atualiza o estado de recepção para ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			// Limpa a flag de interrupção
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    return Status_OK;
  }
  else
  {
    return Status_BUSY;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_SPI_Transmit
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  uint32_t TimeoutCnt = 0;

  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			 // Limpa a flag antes da transmissão
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

    while(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize) // Verifica se todos os dados foram recebidos
    {
      if((US2CON1 & USCI2_SPI_DATA16) != 0) /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      {
        USCI2_SPI_SendData_16(*USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount);			// Transmite 16 bits e incrementa o endereço dos dados
      }
      else/**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      {
        USCI2_SPI_SendData_8(*USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount);			// Transmite 8 bits e incrementa o endereço dos dados
      }
      while(!USCI2_GetFlagStatus(USCI2_SPI_FLAG_SPIF))			// Aguarda a conclusão da transmissão
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        if(TimeoutCnt++ > Timeout)
        {
          USCI2_HANDLE->TxState = USCI2_STATE_TIMEOUT;// Timeout de transmissão
          return Status_TIMEOUT;
        }
        WDTCON |= 0x10;			// Alimenta o watchdog para evitar reset durante uma espera longa
      }
      USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);				// Limpa a flag antes de iniciar a transmissão
      TimeoutCnt = 0;			// Zera o contador de timeout
      USCI2_HANDLE->TxXferCount++;	// Atualiza a contagem de dados transmitidos
    }
    if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
    {
      USCI2_HANDLE->TxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;// Retorna o estado ocupado
  }
}

/**
 * Função: USCI2_SPI_Transmit_IT
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{

  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			// Limpa a flag de interrupção
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = 0;			// Define o contador de transmissão como zero

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((US2CON1 & USCI2_SPI_DATA16) != 0)
    {
      USCI2_SPI_SendData_16(*(USCI2_HANDLE->pTxBuffPtr.Size_u16));
    }
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    else
    {
      USCI2_SPI_SendData_8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8));
    }

    return Status_OK;
  }
  else
  {
    return  Status_BUSY;			// Atualiza o estado para ocupado
  }
}

/**
 * Função: USCI2_SPI_TransmitReceive
 * Descrição: Realiza transmissão e recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_TransmitReceive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pTxData, uint8_t* pRxData, uint8_t Size, uint32_t Timeout)
{
  uint32_t TimeoutCnt = 0;

  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if((USCI2_HANDLE->TxState == USCI2_STATE_READY) && (USCI2_HANDLE->RxState == USCI2_STATE_READY))
  {

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return  Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			// Limpa a flag de interrupção
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pTxData;       // Ponteiro para os dados a transmitir
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pRxData;
    USCI2_HANDLE->TxXferSize = USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    while(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize);
    {

      if((US2CON1 & USCI2_SPI_DATA16) != 0)
      {
        USCI2_SPI_SendData_16(*(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount));			// Transmite 16 bits e incrementa o endereço do buffer
        while(!USCI2_GetFlagStatus(USCI2_SPI_FLAG_SPIF))			// Aguarda a conclusão da transmissão
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          if(TimeoutCnt++ > Timeout)
          {
            USCI2_HANDLE->RxState = USCI2_STATE_TIMEOUT;// Erro de recepção
            return Status_TIMEOUT;				// Retorna erro de timeout
          }
          WDTCON |= 0x10;			// Alimenta o watchdog para evitar reset durante uma espera longa
        }
        USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);				// Limpa a flag antes de iniciar a transmissão
        *(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount) = USCI2_SPI_ReceiveData_16();
      }
      else
      {
        USCI2_SPI_SendData_8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount));			// Transmite 16 bits e incrementa o endereço do buffer
        while(!USCI2_GetFlagStatus(USCI2_SPI_FLAG_SPIF))			// Aguarda a conclusão da transmissão
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          if(TimeoutCnt++ > Timeout)
          {
            USCI2_HANDLE->RxState = USCI2_STATE_TIMEOUT;// Erro de recepção
            return Status_TIMEOUT;				// Retorna erro de timeout
          }
          WDTCON |= 0x10;			// Alimenta o watchdog para evitar reset durante uma espera longa
        }
        USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);				// Limpa a flag antes de iniciar a transmissão
        *(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount) = USCI2_SPI_ReceiveData_8();
      }
      TimeoutCnt = 0;			// Zera o contador de timeout
      USCI2_HANDLE->TxXferCount++;			// Incrementa o contador de dados
      USCI2_HANDLE->TxXferCount++;			// Incrementa o contador de dados
    }
    if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
    {
      USCI2_HANDLE->TxState = USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_ERROR;
  }
}


/**
 * Função: USCI2_SPI_TransmitReceive_IT
 * Descrição: Realiza transmissão e recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_SPI_TransmitReceive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pTxData, uint8_t* pRxData, uint8_t Size)
{
  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if((USCI2_HANDLE->TxState == USCI2_STATE_READY) && (USCI2_HANDLE->RxState == USCI2_STATE_READY))
  {
    Select_USCI2();// Seleciona o USCI2 para configuração/controle

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 == 0) || (Size == 0U))
    {
      return  Status_ERROR;
    }

    USCI2_SPI_Cmd(DISABLE);
    USCI2_HANDLE->TxState = USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_SPI_FLAG_SPIF);			// Limpa a flag de interrupção
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pTxData;
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pRxData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão
    USCI2_SPI_Cmd(ENABLE);

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((US2CON1 & USCI2_SPI_DATA16) != 0)
    {
      USCI2_SPI_SendData_16(*(USCI2_HANDLE->pTxBuffPtr.Size_u16));			// Transmite dados de 16 bits
    }
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    else
    {
      USCI2_SPI_SendData_8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8));			// Transmite dados de 8 bits
    }
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */

    return  Status_OK;
  }
  else
  {
    return Status_ERROR;
  }
}

/*************** Funções TWI ***************/
/**
 * Função: USCI2_TWI_Slave_Init
 * Descrição: Inicializa e configura o periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_Slave_Init(uint8_t TWI_Address)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI
  US2CON2 = TWI_Address << 1;
}

/**
 * Função: USCI2_TWI_MasterCommunicationRate
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_MasterCommunicationRate(USCI2_TWI_MasterCommunicationRate_TypeDef
                                       TWI_MasterCommunicationRate)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI

  US2CON1 = TWI_MasterCommunicationRate;
}

/**
 * Função: USCI2_TWI_Start
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_Start(void)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  US2CON1 |= 0x20;
}

/**
 * Função: USCI2_TWI_MasterModeStop
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_MasterModeStop(void)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  US2CON1 |= 0x10;
}

/**
 * Função: USCI2_TWI_SlaveClockExtension
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_SlaveClockExtension(FunctionalState NewState)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI

  if(NewState != DISABLE)
  {
    US2CON1 |= 0x40;
  }
  else
  {
    US2CON1 &= 0XBF;
  }
}

/**
 * Função: USCI2_TWI_AcknowledgeConfig
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_AcknowledgeConfig(FunctionalState NewState)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI

  if(NewState != DISABLE)
  {
    US2CON0 |= 0X08;
  }
  else
  {
    US2CON0 &= 0XF7;
  }
}

/**
 * Função: USCI2_TWI_GeneralCallCmd
 * Descrição: Habilita ou desabilita a função do periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_GeneralCallCmd(FunctionalState NewState)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI

  if(NewState != DISABLE)
  {
    US2CON2 |= 0X01;
  }
  else
  {
    US2CON2 &= 0XFE;
  }
}

/**
 * Função: USCI2_GetTWIStatus
 * Descrição: Consulta o estado ou a informação solicitada.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
FlagStatus USCI2_GetTWIStatus(USCI2_TWIState_TypeDef USCI2_TWIState)
{
  if((US2CON0 & 0x07) == USCI2_TWIState)
    return SET;
  else
    return RESET;
}

/**
 * Função: USCI2_TWI_Cmd
 * Descrição: Habilita ou desabilita a função do periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_Cmd(FunctionalState NewState)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetTWI();// Configura o USCI2 no modo TWI

  if(NewState != DISABLE)
  {
    US2CON0 |= 0X80;
  }
  else
  {
    US2CON0 &= (~0X80);
  }
}

/**
 * Função: USCI2_TWI_SendAddr
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_SendAddr(uint8_t Addr, USCI2_TWI_RWType RW)
{
  US2CON3 = (Addr << 1) | RW;
}


/**
 * Função: USCI2_TWI_SendData
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_TWI_SendData(uint8_t Data)
{
  US2CON3 = Data;
}


/**
 * Função: USCI2_TWI_ReceiveData
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
uint8_t USCI2_TWI_ReceiveData(void)
{
  return US2CON3;
}

/**
 * Função: USCI2_TWI_Wait_TWIF
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Wait_TWIF(USCI2_HandleInfoDef* USCI2_HANDLE, uint32_t Timeout)
{
  uint32_t TimeoutCnt = 0;
  while(!(USCI2_GetFlagStatus(USCI2_TWI_FLAG_TWIF)))			// Aguarda a conclusão do envio da condição de início
  {
    TimeoutCnt++;
    if(TimeoutCnt > Timeout)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if(USCI2_HANDLE->TxState == USCI2_STATE_BUSY)
        USCI2_HANDLE->TxState = USCI2_STATE_TIMEOUT;
      if(USCI2_HANDLE->RxState == USCI2_STATE_BUSY)
        USCI2_HANDLE->RxState = USCI2_STATE_TIMEOUT;
      return Status_TIMEOUT;
    }
  }
  USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			// Limpa a flag
  return Status_OK;
}

/**
 * Função: USCI2_TWI_Master_Transmit
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x01 != 0) || (Size == 0U))
    {
      return Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

    USCI2_TWI_Start();			// Envia a condição de início
    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)           // Aguarda a conclusão do envio da condição de início
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_MasterModeStop();
      return Status_TIMEOUT;
    }

    if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendAddress) == RESET)          // Verifica o estado da máquina de estados
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_MasterModeStop();
      return Status_ERROR;
    }

    US2CON3 = (slaveAddr << 1) & 0xFE;			// Envia o endereço e o bit de leitura/escrita
    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)           // Aguarda a conclusão do envio do sinal
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_MasterModeStop();
      return Status_TIMEOUT;
    }

    do
    {
      if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendData) == RESET)          // Verifica o estado da máquina de estados
      {
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        return Status_ERROR;
      }
      US1CON3 = *(USCI2_HANDLE->pTxBuffPtr.Size_u8  + USCI2_HANDLE->TxXferCount);             // Transmite dados TWI
      if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)           // Aguarda a conclusão do envio do sinal
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        return Status_TIMEOUT;
      }
      USCI2_HANDLE->TxXferCount++;
    }
    while(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize);

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    USCI2_TWI_MasterModeStop();

    if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
    {
      USCI2_HANDLE->TxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_TWI_Master_Transmit_IT
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size)
{
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    else
    {
      Select_USCI2();
      IE2 &= ~0x02;			// Desabilita a interrupção

      USCI2_HANDLE->TxState = USCI2_STATE_WAIT;			// Processo de transmissão ocupado
      USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
      USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
      USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
      USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

      USCI2_TWI_Start();			// Envia a condição de início

      if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, 0xFFFFFFFF) == Status_TIMEOUT)            // Aguarda a conclusão do envio da condição de início
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        IE2 |= 0x02;	// Habilita a interrupção
        return Status_TIMEOUT;
      }

      if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendAddress) == RESET)           // Verifica o estado da máquina de estados
      {
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        IE2 |= 0x02;	// Habilita a interrupção
        return Status_ERROR;
      }

      IE2 |= 0x02;						// Habilita a interrupção
      US2CON3 = (slaveAddr << 1) & 0xFE;// Envia o endereço e o bit de leitura/escrita

      return Status_OK;
    }
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_TWI_Master_Transmit_IRQHandler
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  if(USCI2_HANDLE->TxState == USCI2_STATE_BUSY)
  {
    if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendData) == SET)
    {
      if(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize)
      {
        USCI2_HANDLE->TxXferCount++;			// O quadro de endereço também é contabilizado como dado
        US2CON3 = *(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount);             // Transmite dados TWI
        return Status_BUSY;
      }
      else if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        USCI2_HANDLE->TxState = USCI2_STATE_READY;
        return Status_OK;
      }
      else
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_MasterModeStop();
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        return Status_OK;
      }
    }
    else if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaUACK) == SET)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_HANDLE->TxXferCount++;
      USCI2_TWI_MasterModeStop();
      if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
      {
        USCI2_HANDLE->TxState = USCI2_STATE_READY;
        return Status_OK;
      }
      else
      {
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        return Status_ERROR;
      }
    }
    else
    {
      USCI2_TWI_MasterModeStop();
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
      return Status_ERROR;
    }
  }
  else if(USCI2_HANDLE->TxState == USCI2_STATE_WAIT)
  {
    // Endereço reconhecido com sucesso
    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;
    US2CON3 = *(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount);
    return Status_BUSY;
  }
  else
  {
    return Status_ERROR;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_TWI_Slave_Transmit_IT
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Slave_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{
  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    else
    {
      Select_USCI2();

      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Habilita o bit AA
      USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
      USCI2_TWI_SlaveClockExtension(ENABLE);            // Habilita o prolongamento do clock
      USCI2_HANDLE->TxState = USCI2_STATE_WAIT;			// Processo de transmissão ocupado
      USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
      USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
      USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

      USCI2_TWI_AcknowledgeConfig(ENABLE);         // Habilita o bit AA

      return Status_OK;;
    }
  }
  else
  {
    return Status_ERROR;
  }
}

/**
 * Função: USCI2_TWI_Slave_Transmit_IRQHandler
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Slave_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  if(USCI2_HANDLE->TxState == USCI2_STATE_BUSY)
  {

    if(USCI2_GetTWIStatus(USCI2_TWI_SlaveSendData) == SET)
    {
      if(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize)
      {
        USCI2_HANDLE->TxXferCount++;			// A primeira interrupção indica que o endereço foi recebido
        US2CON3 = *(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount);
        if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize - 1)
        {
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        }
        return Status_BUSY;
      }
      else
      {
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        return Status_ERROR;
      }
    }
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    else if((USCI2_GetTWIStatus(USCI2_TWI_SlaveDisableACK) == SET) || (USCI2_GetTWIStatus(USCI2_TWI_SlaveReceivedaUACK) == SET))
    {
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      USCI2_HANDLE->TxXferCount++;
      if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)			// Todos os dados foram transmitidos
      {
        USCI2_HANDLE->TxState = USCI2_STATE_READY;
        return Status_OK;
      }
      else
      {
        USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
        return Status_ERROR;
      }
    }
    else
    {
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
      return Status_ERROR;
    }
  }
  else if(USCI2_HANDLE->TxState == USCI2_STATE_WAIT)
  {
    if(USCI2_GetTWIStatus(USCI2_TWI_SlaveSendData) == SET)
    {
      US2CON3 = *(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount);
      USCI2_HANDLE->TxState = USCI2_STATE_BUSY;
    }
    return Status_BUSY;
  }
  else
  {
    return Status_ERROR;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_TWI_Master_Receive
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    Select_USCI2();
    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
    USCI2_TWI_AcknowledgeConfig(ENABLE);         // Habilita o bit AA
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    USCI2_TWI_Start();			// Envia a condição de início
    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio da condição de início
    {
      USCI2_TWI_AcknowledgeConfig(DISABLE);
      return Status_TIMEOUT;
    }

    if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendAddress) == RESET)           // Verifica o estado da máquina de estados
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
      USCI2_TWI_AcknowledgeConfig(DISABLE);
      return Status_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    US2CON3 = (slaveAddr << 1) | 0x01;// Envia o endereço e o bit de leitura/escrita
    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio do sinal
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      USCI2_TWI_MasterModeStop();
      return Status_TIMEOUT;
    }

    if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaData) == RESET)           // Verifica o estado da máquina de estados
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      USCI2_TWI_MasterModeStop();
      return Status_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    do
    {
      if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio do sinal
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        USCI2_TWI_MasterModeStop();
        return Status_TIMEOUT;
      }

      if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaData) == RESET)           // Verifica o estado da máquina de estados
      {
        USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        USCI2_TWI_MasterModeStop();
        return Status_ERROR;
      }

      *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;             // Recebe dados TWI
      USCI2_HANDLE->RxXferCount++;
      if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize - 1)
      {
        USCI2_TWI_AcknowledgeConfig(DISABLE);                                     // Desabilita o bit AA
        if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio do sinal
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
          USCI2_TWI_MasterModeStop();
          return Status_TIMEOUT;
        }
        if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaUACK) == RESET)           // Verifica o estado da máquina de estados
        {
          USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
          USCI2_TWI_MasterModeStop();
          return Status_ERROR;
        }

        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;             // Recebe dados TWI
        USCI2_HANDLE->RxXferCount++;
      }
    }
    while(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize);
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
    USCI2_TWI_MasterModeStop();

    if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
    {
      USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;
  }
}


/**
 * Função: USCI2_TWI_Slave_Receive
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Slave_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    Select_USCI2();
    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
    USCI2_TWI_AcknowledgeConfig(ENABLE);         // Habilita o bit AA
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio do sinal
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      return Status_TIMEOUT;
    }

    if(USCI2_GetTWIStatus(USCI2_TWI_SlaveReceivedaData) == RESET)           // Verifica o estado da máquina de estados
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      return Status_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    do
    {
      if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, Timeout) == Status_TIMEOUT)            // Aguarda a conclusão do envio da condição de início
      {
        return Status_TIMEOUT;
      }

      if(USCI2_GetTWIStatus(USCI2_TWI_SlaveReceivedaData) == SET)           // Verifica o estado da máquina de estados
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;             // Recebe dados TWI
        USCI2_HANDLE->RxXferCount++;
        if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize - 1)
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita AA e envia resposta UAC ao mestre
      }
      else if(USCI2_GetTWIStatus(USCI2_TWI_SlaveIdle) == SET)
      {
        if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize - 1)
        {
          *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;
          USCI2_HANDLE->RxXferCount++;
        }
        else
        {
          USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
          return Status_ERROR;
        }
      }
      else
      {
        USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        return Status_ERROR;
      }

    }
    while(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize);

    if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
    {
      USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_TWI_Master_Receive_IT
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t slaveAddr, uint8_t* pData, uint8_t Size)
{
  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    Select_USCI2();
    IE2 &= ~0x02;	// Desabilita a interrupção
    USCI2_HANDLE->RxState = USCI2_STATE_WAIT;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão
    USCI2_TWI_AcknowledgeConfig(ENABLE);         // Habilita o bit AA
    USCI2_TWI_Start();			// Envia a condição de início

    if(USCI2_TWI_Wait_TWIF(USCI2_HANDLE, 0xFFFF) == Status_TIMEOUT)            // Aguarda a conclusão do envio da condição de início
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      USCI2_TWI_MasterModeStop();
      IE2 |= 0x02;	// Habilita a interrupção
      return Status_TIMEOUT;
    }

    if(USCI2_GetTWIStatus(USCI2_TWI_MasterSendAddress) == RESET)           // Verifica o estado da máquina de estados
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      USCI2_TWI_MasterModeStop();
      IE2 |= 0x02;	// Habilita a interrupção
      return Status_ERROR;
    }

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    IE2 |= 0x02;	// Habilita a interrupção
    US2CON3 = (slaveAddr << 1) | 0x01;// Envia o endereço e o bit de leitura/escrita

    return Status_OK;
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_TWI_Master_Receive_IRQHandler
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Master_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  if(USCI2_HANDLE->RxState == USCI2_STATE_BUSY)
  {
    if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaData) == SET)           // Verifica o estado da máquina de estados
    {
      if(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;             // Recebe dados TWI
        USCI2_HANDLE->RxXferCount++;
        if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
          USCI2_TWI_MasterModeStop();
          USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Transmissão concluída
          return Status_OK;
        }
        return Status_BUSY;
      }
      else
      {
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        USCI2_TWI_MasterModeStop();
        USCI2_HANDLE->RxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
        return Status_ERROR;
      }
    }
  }
  else if(USCI2_HANDLE->RxState == USCI2_STATE_WAIT)
  {
    if(USCI2_GetTWIStatus(USCI2_TWI_MasterReceivedaData) == SET)
    {
      USCI2_HANDLE->RxState = USCI2_STATE_BUSY;
    }
    return Status_BUSY;
  }
  return Status_ERROR;
}


/**
 * Função: USCI2_TWI_Slave_Receive_IT
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Slave_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }
    Select_USCI2();
    USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
    USCI2_ClearFlag(USCI2_TWI_FLAG_TWIF);			 // Limpa a flag antes da transmissão
    USCI2_HANDLE->RxState = USCI2_STATE_WAIT;			// Processo de transmissão ocupado
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão
    USCI2_TWI_AcknowledgeConfig(ENABLE);         // Habilita o bit AA

    return Status_OK;
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_TWI_Slave_Receive_IRQHandler
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_TWI_Slave_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  if(USCI2_HANDLE->RxState == USCI2_STATE_BUSY)
  {
    if(USCI2_GetTWIStatus(USCI2_TWI_SlaveReceivedaData) == SET)           // Verifica o estado da máquina de estados
    {
      if(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;             // Transmite dados TWI
        USCI2_HANDLE->RxXferCount++;
        if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize - 1)
        {
          USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita AA e envia resposta UACK ao mestre
        }
      }
      else
      {
        USCI2_TWI_AcknowledgeConfig(DISABLE);
        return Status_ERROR;
      }
      return Status_BUSY;
    }
    else if(USCI2_GetTWIStatus(USCI2_TWI_SlaveIdle) == SET)
    {
      if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize - 1)
      {
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = US2CON3;
        USCI2_HANDLE->RxXferCount++;
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita AA e envia resposta UACK ao mestre
        USCI2_HANDLE->RxState = USCI2_STATE_READY;
        return Status_OK;
      }
      else
      {
        USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
        USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
        return Status_ERROR;
      }
    }
    else
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
      USCI2_TWI_AcknowledgeConfig(DISABLE);         // Desabilita o bit de habilitação AA
      return Status_ERROR;
    }
  }
  else if(USCI2_HANDLE->RxState == USCI2_STATE_WAIT)
  {
    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;
    return Status_BUSY;
  }
  else
  {
    return Status_ERROR;
  }
}

/*************** Funções UART ***************/
/**
 * Função: USCI2_UART_Init
 * Descrição: Inicializa e configura o periférico.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_UART_Init(uint32_t UARTFsys, uint32_t BaudRate, USCI2_UART_Mode_TypeDef Mode,
                     USCI2_UART_RX_TypeDef RxMode)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle
  USCI2_SetUART();// Configura o USCI2 no modo UART
  US2CON0 = US2CON0 & 0X0F | Mode | RxMode;

  if(Mode == USCI2_UART_Mode_8B)
  {
    if(BaudRate == USCI2_UART_BaudRate_FsysDIV12)
    {
      US2CON0 &= 0XDF;
    }
    else if(BaudRate == USCI2_UART_BaudRate_FsysDIV4)
    {
      US2CON0 |= 0X20;
    }
  }
  else
  {
    US2CON2 = UARTFsys / BaudRate / 256;
    US2CON1 = UARTFsys / BaudRate % 256;
  }
}

/**
 * Função: USCI2_UART_SendData8
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_UART_SendData8(uint8_t Data)
{
  US2CON3 = Data;
}

/**
 * Função: USCI2_UART_ReceiveData8
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
uint8_t USCI2_UART_ReceiveData8(void)
{
  return US2CON3;
}

/**
 * Função: USCI2_UART_SendData9
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_UART_SendData9(uint16_t Data)
{
  uint8_t Data_9Bit;
  Data_9Bit = (Data >> 8);

  if(Data_9Bit)
  {
    US2CON0 |= 0x08;
  }
  else
  {
    US2CON0 &= 0xf7;
  }

  US2CON3 = (uint8_t)Data;
}

/**
 * Função: USCI2_UART_ReceiveData9
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
uint16_t USCI2_UART_ReceiveData9(void)
{
  uint16_t Data9;
  Data9 = US2CON3 + ((uint16_t)(US2CON0 & 0X04) << 6);
  return Data9;
}

/**
 * Função: USCI2_UART_Transmit
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Transmit(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{

  uint32_t delaytime = 0;
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U))
    {
      return  Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_UART_FLAG_TI); // Limpa a flag de interrupção de transmissão antes de transmitir
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

    while(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize) // Verifica se todos os dados foram recebidos
    {
      if((US2CON0 & 0xC0) == USCI2_UART_Mode_11B)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_UART_SendData9(*(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount));
      }
      else
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_UART_SendData8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount));
      }

      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      delaytime = 0;
      while(!(US2CON0 & 0x02))
      {
        if(delaytime++ > Timeout)
        {
          USCI2_HANDLE->TxState = USCI2_STATE_TIMEOUT;// Timeout de transmissão
          return Status_TIMEOUT;
        }
      }

      USCI2_ClearFlag(USCI2_UART_FLAG_TI); // Limpa a flag de interrupção de transmissão (escrita de 1 ou 0, conforme o dispositivo)
      USCI2_HANDLE->TxXferCount++;	// Atualiza a contagem de dados transmitidos
    }

    if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
    {
      USCI2_HANDLE->TxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_UART_Transmit_IT
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Transmit_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U))
    {
      return Status_ERROR;
    }

    USCI2_HANDLE->TxState = USCI2_STATE_BUSY;
    USCI2_ClearFlag(USCI2_UART_FLAG_TI); /* Limpa uma flag TI pendente antes de iniciar a transmissão. */
    USCI2_HANDLE->pTxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->TxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->TxXferCount = 0;			// Zera o contador de transmissão

    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((US2CON0 & 0xC0) == USCI2_UART_Mode_11B)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_UART_SendData9(*(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount));
    }
    else
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_UART_SendData8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount));
    }
    return Status_OK;
  }
  else
  {
    return Status_BUSY;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_UART_Transmit_IRQHandler
 * Descrição: Realiza a transmissão de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Transmit_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->TxState == USCI2_STATE_BUSY)
  {
    USCI2_HANDLE->TxXferCount++;
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(USCI2_HANDLE->TxXferCount < USCI2_HANDLE->TxXferSize)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if((US2CON0 & 0xC0) == USCI2_UART_Mode_11B)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_UART_SendData9(*(USCI2_HANDLE->pTxBuffPtr.Size_u16 + USCI2_HANDLE->TxXferCount));
      }
      else
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_UART_SendData8(*(USCI2_HANDLE->pTxBuffPtr.Size_u8 + USCI2_HANDLE->TxXferCount));
      }
      return Status_OK;
    }
    else if(USCI2_HANDLE->TxXferCount == USCI2_HANDLE->TxXferSize)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_HANDLE->TxState = USCI2_STATE_READY;
      return Status_OK;
    }
    else
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_HANDLE->TxState = USCI2_STATE_ERROR;
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_UART_Receive
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Receive(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size, uint32_t Timeout)
{
  uint32_t delaytime = 0;
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if((IE2 & 0x02 != 0) || (Size == 0U) || ((US2CON0 & 0x10) != USCI2_UART_RX_ENABLE))
    {
      return  Status_ERROR;
    }

    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;			// Processo de transmissão ocupado
    USCI2_ClearFlag(USCI2_UART_FLAG_RI);			 // Limpa a flag de recepção antes de receber
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para os dados a transmitir
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a transmitir
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    while(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)  // Verifica se todos os dados foram recebidos
    {
      if(USCI2_GetFlagStatus(USCI2_UART_FLAG_RI))  // Verifica a flag de recepção
      {
        USCI2_ClearFlag(USCI2_UART_FLAG_RI);			// Limpa a flag de recepção

        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        if((US2CON0 & 0xC0) == USCI2_UART_Mode_11B)
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          *(USCI2_HANDLE->pRxBuffPtr.Size_u16 + USCI2_HANDLE->RxXferCount) = USCI2_UART_ReceiveData9();
        }
        else
        {
          /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
          *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = USCI2_UART_ReceiveData8();
        }

        USCI2_HANDLE->RxXferCount++;	// Atualiza a contagem de dados recebidos
        delaytime = 0;// Dado recebido; zera o contador de timeout
      }
      if(delaytime++ > Timeout) // Estouro do contador de tempo
      {
        USCI2_HANDLE->RxState = USCI2_STATE_TIMEOUT;// Timeout de transmissão
        return Status_TIMEOUT;
      }
    }

    if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
    {
      USCI2_HANDLE->RxState = USCI2_STATE_READY;	// Transmissão concluída
      return Status_OK;
    }
    else
    {
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;// Ocorreu um erro na transmissão
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_UART_Receive_IT
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Receive_IT(USCI2_HandleInfoDef* USCI2_HANDLE, uint8_t* pData, uint8_t Size)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_READY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(((IE2 & 0x02) == 0) || (Size == 0U) || ((US2CON0 & 0x10) != USCI2_UART_RX_ENABLE))
    {
      return Status_ERROR;
    }

    USCI2_HANDLE->RxState = USCI2_STATE_BUSY;
    USCI2_HANDLE->pRxBuffPtr.Size_u8 = pData;			// Ponteiro para o buffer de recepção
    USCI2_HANDLE->RxXferSize = Size;			// Atualiza a quantidade de dados a receber
    USCI2_HANDLE->RxXferCount = 0;			// Zera o contador de transmissão

    return Status_OK;
  }
  else
  {
    return Status_BUSY;// Retorna o estado da operação
  }
}

/**
 * Função: USCI2_UART_Receive_IRQHandler
 * Descrição: Realiza a recepção de dados.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
StatusTypeDef USCI2_UART_Receive_IRQHandler(USCI2_HandleInfoDef* USCI2_HANDLE)
{
  Select_USCI2();// Seleciona o USCI2 para configuração/controle

  /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
  if(USCI2_HANDLE->RxState == USCI2_STATE_BUSY)
  {
    /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
    if(USCI2_HANDLE->RxXferCount < USCI2_HANDLE->RxXferSize)
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if((US2CON0 & 0xC0) == USCI2_UART_Mode_11B)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        *(USCI2_HANDLE->pRxBuffPtr.Size_u16 + USCI2_HANDLE->RxXferCount) = USCI2_UART_ReceiveData9();
      }
      else
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        *(USCI2_HANDLE->pRxBuffPtr.Size_u8 + USCI2_HANDLE->RxXferCount) = USCI2_UART_ReceiveData8();
      }
      USCI2_HANDLE->RxXferCount++;
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      if(USCI2_HANDLE->RxXferCount == USCI2_HANDLE->RxXferSize)
      {
        /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
        USCI2_HANDLE->RxState = USCI2_STATE_READY;
      }

      return Status_OK;
    }
    else
    {
      /**
 * Função: USCI2
 * Descrição: Configura ou executa a operação indicada pelo nome da função.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
      USCI2_HANDLE->RxState = USCI2_STATE_ERROR;
      return Status_ERROR;
    }
  }
  else
  {
    return Status_BUSY;
  }
}

/**
 * Função: USCI2_GetFlagStatus
 * Descrição: Consulta o estado ou a informação solicitada.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
FlagStatus USCI2_GetFlagStatus(USCI2_Flag_TypeDef USCI2_FLAG)
{
  FlagStatus bitstatus = RESET;
  Select_USCI2();
	if((TMCON & 0xC0)==0x40)  // Modo SPI
	{
#if defined(SC95F8x1x) || defined(SC95F7x1x) || defined(SC95FWxx)	
		
	  if((USCI2_FLAG == USCI2_SPI_FLAG_SPIF) || (USCI2_FLAG == USCI2_SPI_FLAG_WCOL))
#else	
	  if((USCI2_FLAG == USCI2_SPI_FLAG_SPIF) || (USCI2_FLAG == USCI2_SPI_FLAG_WCOL) || (USCI2_FLAG == USCI2_SPI_FLAG_TXE))
#endif
		{
			if((USCI2_FLAG & US2CON1) != (uint8_t)RESET)
      {
        bitstatus = SET;
      }
      else
      {
        bitstatus = RESET;
      }	
	  }
	}else if((TMCON & 0xC0)==0x80)  // Modo TWI
  {
		if(USCI2_FLAG == USCI2_TWI_FLAG_TXRXnE)
		{
			if((USCI2_FLAG & US2CON1) != (uint8_t)RESET)
      {
        bitstatus = SET;
      }
      else
      {
        bitstatus = RESET;
      }
		}
		else
    {
			if((USCI2_FLAG & US2CON0) != (uint8_t)RESET)
      {
        bitstatus = SET;
      }
      else
      {
        bitstatus = RESET;
      }
    } 
  }else if((TMCON & 0xC0)==0xC0)  // Modo UART
  {
    if((USCI2_FLAG & US2CON0) != (uint8_t)RESET)
    {
      bitstatus = SET;
    }
    else
    {
      bitstatus = RESET;
    }
  }
  return bitstatus;
}

/**
 * Função: USCI2_ClearFlag
 * Descrição: Limpa a flag ou o estado indicado.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_ClearFlag(USCI2_Flag_TypeDef USCI2_FLAG)
{
  Select_USCI2();
	if((TMCON & 0xC0)==0x40)  // Modo SPI
	{
#if defined(SC95F8x1x) || defined(SC95F7x1x) || defined(SC95FWxx)	
		
	  if((USCI2_FLAG == USCI2_SPI_FLAG_SPIF) || (USCI2_FLAG == USCI2_SPI_FLAG_WCOL))
#else	
	  if((USCI2_FLAG == USCI2_SPI_FLAG_SPIF) || (USCI2_FLAG == USCI2_SPI_FLAG_WCOL) || (USCI2_FLAG == USCI2_SPI_FLAG_TXE))
#endif
		{
		  US2CON1 &= (~USCI2_FLAG); // Acessa o registrador US2CON1
		}
	}else if((TMCON & 0xC0)==0x80)  // Modo TWI
	{
	  if(USCI2_FLAG == USCI2_TWI_FLAG_TXRXnE)
    {
      US2CON1 &= (~USCI2_FLAG); // Acessa o registrador US2CON1
    }
		else
    {
       US2CON0 &= (~USCI2_FLAG); // Acessa o registrador US2CON0
    }	
	}else if((TMCON & 0xC0)==0xC0)  // Modo UART
	{
	  if((USCI2_FLAG == USCI2_UART_FLAG_TI) || (USCI2_FLAG == USCI2_UART_FLAG_RI))
    {
#if defined(SC95F8x3x) || defined(SC95F7x3x) || defined (SC95F8x6x) || defined (SC95F7x6x)  || defined (SC95F8x1xB) || defined (SC95F7x1xB)\
		|| defined (SC95R602)  || defined (SC95R605) || defined (SC95F8x7x) || defined (SC95F7x7x) || defined (SC95F7x0x)
     US2CON0 = US2CON0  & 0xFC | USCI2_FLAG;// Limpa escrevendo 1
#else
     US2CON0 &= (~USCI2_FLAG); // Limpa escrevendo 0
#endif
    }	
	}
}

/**
 * Função: USCI2_ITConfig
 * Descrição: Configura a habilitação e a prioridade da interrupção.
 * Observação: os parâmetros e o estado de retorno seguem a assinatura da função.
 */
void USCI2_ITConfig(FunctionalState NewState, PriorityStatus Priority)
{
  if(NewState != DISABLE)
  {
    IE2 |= 0x02;
  }
  else
  {
    IE2 &= 0xFD;
  }

  /************************************************************/
  if(Priority != LOW)
  {
    IP2 |= 0x02;
  }
  else
  {
    IP2 &= 0xFD;
  }
}


/******************* (C) COPYRIGHT 2022 SinOne Microelectronics *****END OF FILE****/
