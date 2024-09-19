
#ifndef _MB_PORT_H
#define _MB_PORT_H

#include <stdint.h>

/* ----------------------- Type definitions ---------------------------------*/

typedef enum
{
    EV_READY            = 1<<0,         /*!< Startup finished. */
    EV_FRAME_RECEIVED   = 1<<1,         /*!< Frame received. */
    EV_EXECUTE          = 1<<2,         /*!< Execute function. */
    EV_FRAME_SENT       = 1<<3          /*!< Frame sent. */
} eMBEventType;

typedef enum
{
    EV_MASTER_READY                    = 1<<0,  /*!< Startup finished. */
    EV_MASTER_FRAME_RECEIVED           = 1<<1,  /*!< Frame received. */
    EV_MASTER_EXECUTE                  = 1<<2,  /*!< Execute function. */
    EV_MASTER_FRAME_SENT               = 1<<3,  /*!< Frame sent. */
    EV_MASTER_ERROR_PROCESS            = 1<<4,  /*!< Frame error process. */
    EV_MASTER_PROCESS_SUCCESS          = 1<<5,  /*!< Request process success. */
    EV_MASTER_ERROR_RESPOND_TIMEOUT    = 1<<6,  /*!< Request respond timeout. */
    EV_MASTER_ERROR_RECEIVE_DATA       = 1<<7,  /*!< Request receive data error. */
    EV_MASTER_ERROR_EXECUTE_FUNCTION   = 1<<8,  /*!< Request execute function error. */
} eMBMasterEventType;

typedef enum
{
    EV_ERROR_RESPOND_TIMEOUT,         /*!< Slave respond timeout. */
    EV_ERROR_RECEIVE_DATA,            /*!< Receive frame data erroe. */
    EV_ERROR_EXECUTE_FUNCTION,        /*!< Execute function error. */
} eMBMasterErrorEventType;

/*! \ingroup modbus
 * \brief Parity used for int8_tacters in serial mode.
 *
 * The parity which should be applied to the int8_tacters sent over the serial
 * link. Please note that this values are actually passed to the porting
 * layer and therefore not all parity modes might be available.
 */
typedef enum
{
    MB_PAR_NONE,                /*!< No parity. */
    MB_PAR_ODD,                 /*!< Odd parity. */
    MB_PAR_EVEN                 /*!< Even parity. */
} eMBParity;

/* ----------------------- Supporting functions -----------------------------*/
uint8_t            xMBPortEventInit( void );

uint8_t            xMBPortEventPost( eMBEventType eEvent );

uint8_t            xMBPortEventGet(  /*@out@ */ eMBEventType * eEvent );

uint8_t            xMBMasterPortEventInit( void );

uint8_t            xMBMasterPortEventPost( eMBMasterEventType eEvent );

uint8_t            xMBMasterPortEventGet(  /*@out@ */ eMBMasterEventType * eEvent );

void            vMBMasterOsResInit( void );

uint8_t            xMBMasterRunResTake( int32_t time );

void            vMBMasterRunResRelease( void );

/* ----------------------- Serial port functions ----------------------------*/

uint8_t            xMBPortSerialInit( uint8_t ucPort, uint32_t ulBaudRate,
                                   uint8_t ucDataBits, eMBParity eParity );

void            vMBPortClose( void );

void            xMBPortSerialClose( void );

void            vMBPortSerialEnable( uint8_t xRxEnable, uint8_t xTxEnable );

 uint8_t     xMBPortSerialGetByte( int8_t * pucByte );

 uint8_t     xMBPortSerialPutByte( int8_t ucByte );

uint8_t            xMBMasterPortSerialInit( uint8_t ucPort, uint32_t ulBaudRate,
                                   uint8_t ucDataBits, eMBParity eParity );

void            vMBMasterPortClose( void );

void            xMBMasterPortSerialClose( void );

void            vMBMasterPortSerialEnable( uint8_t xRxEnable, uint8_t xTxEnable );

 uint8_t     xMBMasterPortSerialGetByte( int8_t * pucByte );

 uint8_t     xMBMasterPortSerialPutByte( int8_t ucByte );

/* ----------------------- Timers functions ---------------------------------*/
uint8_t            xMBPortTimersInit( uint16_t usTimeOut50us );

void            xMBPortTimersClose( void );

 void     vMBPortTimersEnable( void );

 void     vMBPortTimersDisable( void );

uint8_t            xMBMasterPortTimersInit( uint16_t usTimeOut50us );

void            xMBMasterPortTimersClose( void );

 void     vMBMasterPortTimersT35Enable( void );

 void     vMBMasterPortTimersConvertDelayEnable( void );

 void     vMBMasterPortTimersRespondTimeoutEnable( void );

 void     vMBMasterPortTimersDisable( void );

/* ----------------- Callback for the master error process ------------------*/
void            vMBMasterErrorCBRespondTimeout( uint8_t ucDestAddress, const uint8_t* pucPDUData,
                                                uint16_t ucPDULength );

void            vMBMasterErrorCBReceiveData( uint8_t ucDestAddress, const uint8_t* pucPDUData,
                                             uint16_t ucPDULength );

void            vMBMasterErrorCBExecuteFunction( uint8_t ucDestAddress, const uint8_t* pucPDUData,
                                                 uint16_t ucPDULength );

void            vMBMasterCBRequestScuuess( void );

/* ----------------------- Callback for the protocol stack ------------------*/

/*!
 * \brief Callback function for the porting layer when a new byte is
 *   available.
 *
 * Depending upon the mode this callback function is used by the RTU or
 * ASCII transmission layers. In any case a call to xMBPortSerialGetByte()
 * must immediately return a new int8_tacter.
 *
 * \return <code>TRUE</code> if a event was posted to the queue because
 *   a new byte was received. The port implementation should wake up the
 *   tasks which are currently blocked on the eventqueue.
 */
extern          uint8_t( *pxMBFrameCBByteReceived ) ( void );

extern          uint8_t( *pxMBFrameCBTransmitterEmpty ) ( void );

extern          uint8_t( *pxMBPortCBTimerExpired ) ( void );

extern          uint8_t( *pxMBMasterFrameCBByteReceived ) ( void );

extern          uint8_t( *pxMBMasterFrameCBTransmitterEmpty ) ( void );

extern          uint8_t( *pxMBMasterPortCBTimerExpired ) ( void );

/* ----------------------- TCP port functions -------------------------------*/
uint8_t            xMBTCPPortInit( uint16_t usTCPPort );

void            vMBTCPPortClose( void );

void            vMBTCPPortDisable( void );

uint8_t            xMBTCPPortGetRequest( uint8_t **ppucMBTCPFrame, uint16_t * usTCPLength );

uint8_t            xMBTCPPortSendResponse( const uint8_t *pucMBTCPFrame, uint16_t usTCPLength );

#endif
