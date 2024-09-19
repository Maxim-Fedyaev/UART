
#include "mbconfig.h"

#ifndef _MB_RTU_H
#define _MB_RTU_H

eMBErrorCode    eMBRTUInit( uint8_t slaveAddress, uint8_t ucPort, uint32_t ulBaudRate,
                            eMBParity eParity );
void            eMBRTUStart( void );
void            eMBRTUStop( void );
eMBErrorCode    eMBRTUReceive( uint8_t * pucRcvAddress, uint8_t ** pucFrame, uint16_t * pusLength );
eMBErrorCode    eMBRTUSend( uint8_t slaveAddress, const uint8_t * pucFrame, uint16_t usLength );
uint8_t            xMBRTUReceiveFSM( void );
uint8_t            xMBRTUTransmitFSM( void );
uint8_t            xMBRTUTimerT15Expired( void );
uint8_t            xMBRTUTimerT35Expired( void );

#if MB_MASTER_RTU_ENABLED > 0
eMBErrorCode    eMBMasterRTUInit( uint8_t ucPort, uint32_t ulBaudRate,eMBParity eParity );
void            eMBMasterRTUStart( void );
void            eMBMasterRTUStop( void );
eMBErrorCode    eMBMasterRTUReceive( uint8_t * pucRcvAddress, uint8_t ** pucFrame, uint16_t * pusLength );
eMBErrorCode    eMBMasterRTUSend( uint8_t slaveAddress, const uint8_t * pucFrame, uint16_t usLength );
uint8_t            xMBMasterRTUReceiveFSM( void );
uint8_t            xMBMasterRTUTransmitFSM( void );
uint8_t            xMBMasterRTUTimerExpired( void );
#endif

#endif
