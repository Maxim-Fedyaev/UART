
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbrtu.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"
#include "hdlcbitstaffing.h"




#if MB_MASTER_RTU_ENABLED > 0

extern void SetKey(uint8_t* key);
extern uint8_t crypto_key[32];

#if HDLC > 0
uint8_t HDLC_Count = 0;
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    STATE_M_RX_INIT,              /*!< Receiver is in initial state. */
    STATE_M_RX_IDLE,              /*!< Receiver is in idle state. */
    STATE_M_RX_RCV,               /*!< Frame is beeing received. */
    STATE_M_RX_ERROR,              /*!< If the frame is invalid. */
} eMBMasterRcvState;

typedef enum
{
    STATE_M_TX_IDLE,              /*!< Transmitter is in idle state. */
    STATE_M_TX_XMIT,              /*!< Transmitter is in transfer state. */
    STATE_M_TX_XFWR,              /*!< Transmitter is in transfer finish and wait receive state. */
} eMBMasterSndState;

/* ----------------------- Static variables ---------------------------------*/
static volatile eMBMasterSndState eSndState;
static volatile eMBMasterRcvState eRcvState;

static volatile uint8_t  ucMasterRTUSndBuf[MB_PDU_SIZE_MAX];
static volatile uint8_t  ucMasterRTURcvBuf[MB_SER_PDU_SIZE_MAX];
static volatile uint16_t usMasterSendPDULength;

static volatile uint8_t *pucMasterSndBufferCur;
static volatile uint16_t usMasterSndBufferCount;

static volatile uint16_t usMasterRcvBufferPos;
static volatile uint8_t   xFrameIsBroadcast = FALSE;

static volatile eMBMasterTimerMode eMasterCurTimerMode;

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBMasterRTUInit(uint8_t ucPort, uint32_t ulBaudRate, eMBParity eParity )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint32_t           usTimerT35_50us;

    ENTER_CRITICAL_SECTION(  );

    /* Modbus RTU uses 8 Databits. */
    if( xMBMasterPortSerialInit( ucPort, ulBaudRate, 8, eParity ) != TRUE )
    {
        eStatus = MB_EPORTERR;
    }
    else
    {
        /* If baudrate > 19200 then we should use the fixed timer values
         * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
         */
        if( ulBaudRate > 19200 )
        {
            usTimerT35_50us = 35;       /* 1800us. */
        }
        else
        {
            /* The timer reload value for a character is given by:
             *
             * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
             *             = 11 * Ticks_per_1s / Baudrate
             *             = 220000 / Baudrate
             * The reload for t3.5 is 1.5 times this value and similary
             * for t3.5.
             */
            usTimerT35_50us = ( 7UL * 220000UL ) / ( 2UL * ulBaudRate );
        }
        if( xMBMasterPortTimersInit( ( uint16_t ) usTimerT35_50us ) != TRUE )
        {
            eStatus = MB_EPORTERR;
        }
    }    
    #if KUZNECHIK > 0
        SetKey(crypto_key);;
    #endif

    EXIT_CRITICAL_SECTION(  );

    return eStatus;
}

void
eMBMasterRTUStart( void )
{
    ENTER_CRITICAL_SECTION(  );
    /* Initially the receiver is in the state STATE_M_RX_INIT. we start
     * the timer and if no character is received within t3.5 we change
     * to STATE_M_RX_IDLE. This makes sure that we delay startup of the
     * modbus protocol stack until the bus is free.
     */
    eRcvState = STATE_M_RX_INIT;
    vMBMasterPortSerialEnable( TRUE, FALSE );
    vMBMasterPortTimersT35Enable(  );

    EXIT_CRITICAL_SECTION(  );
}

void
eMBMasterRTUStop( void )
{
    ENTER_CRITICAL_SECTION(  );
    vMBMasterPortSerialEnable( FALSE, FALSE );
    vMBMasterPortTimersDisable(  );
    EXIT_CRITICAL_SECTION(  );
}

eMBErrorCode
eMBMasterRTUReceive( uint8_t * pucRcvAddress, uint8_t ** pucFrame, uint16_t * pusLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    ENTER_CRITICAL_SECTION(  );

    #if HDLC > 0
        eStatus = Decoder (ucMasterRTURcvBuf, &usMasterRcvBufferPos, ucMasterRTURcvBuf);
    #endif

    /* Length and CRC check */
    if( ( usMasterRcvBufferPos >= MB_SER_PDU_SIZE_MIN )
        && ( usMBCRC16( ( uint8_t * ) ucMasterRTURcvBuf, usMasterRcvBufferPos ) == 0 ) )
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        *pucRcvAddress = ucMasterRTURcvBuf[MB_SER_PDU_ADDR_OFF];

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = ( uint16_t )( usMasterRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame = ( uint8_t * ) & ucMasterRTURcvBuf[MB_SER_PDU_PDU_OFF];
    }
    else
    {
        eStatus = MB_EIO;
    }

    EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBMasterRTUSend( uint8_t ucSlaveAddress, const uint8_t * pucFrame, uint16_t usLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint16_t          usCRC16;

    if ( ucSlaveAddress > MB_MASTER_TOTAL_SLAVE_NUM ) return MB_EINVAL;

    ENTER_CRITICAL_SECTION(  );

    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if( eRcvState == STATE_M_RX_IDLE )
    {
        /* First byte before the Modbus-PDU is the slave address. */
        pucMasterSndBufferCur = ( uint8_t * ) pucFrame - 1;
        usMasterSndBufferCount = 1;

        /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
        pucMasterSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
        usMasterSndBufferCount += usLength;

        /* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
        usCRC16 = usMBCRC16( ( uint8_t * ) pucMasterSndBufferCur, usMasterSndBufferCount );
        ucMasterRTUSndBuf[usMasterSndBufferCount++] = ( uint8_t )( usCRC16 & 0xFF );
        ucMasterRTUSndBuf[usMasterSndBufferCount++] = ( uint8_t )( usCRC16 >> 8 );

        #if HDLC > 0
            usMasterSndBufferCount = Coder (pucMasterSndBufferCur, usMasterSndBufferCount, pucMasterSndBufferCur);
        #endif

        /* Activate the transmitter. */
        eSndState = STATE_M_TX_XMIT;
        vMBMasterPortSerialEnable( FALSE, TRUE );
    }
    else
    {
        eStatus = MB_EIO;
    }
    EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

uint8_t
xMBMasterRTUReceiveFSM( void )
{
    uint8_t            xTaskNeedSwitch = FALSE;
    uint8_t           ucByte;

    /* Always read the character. */
    ( void )xMBMasterPortSerialGetByte( ( int8_t * ) & ucByte );

    switch ( eRcvState )
    {
        /* If we have received a character in the init state we have to
         * wait until the frame is finished.
         */
    case STATE_M_RX_INIT:
        #if HDLC == 0
        vMBMasterPortTimersT35Enable( );
        #endif
        break;

        /* In the error state we wait until all characters in the
         * damaged frame are transmitted.
         */
    case STATE_M_RX_ERROR:
        #if HDLC == 0
        vMBMasterPortTimersT35Enable( );
        #endif
        break;

        /* In the idle state we wait for a new character. If a character
         * is received the t1.5 and t3.5 timers are started and the
         * receiver is in the state STATE_RX_RECEIVCE and disable early
         * the timer of respond timeout .
         */
    case STATE_M_RX_IDLE:
        /* In time of respond timeout,the receiver receive a frame.
         * Disable timer of respond timeout and change the transmiter state to idle.
         */
        vMBMasterPortTimersDisable( );
        eSndState = STATE_M_TX_IDLE;

        usMasterRcvBufferPos = 0;
        ucMasterRTURcvBuf[usMasterRcvBufferPos++] = ucByte;
        eRcvState = STATE_M_RX_RCV;

        /* Enable t3.5 timers. */
        vMBMasterPortTimersT35Enable( );
        break;

        /* We are currently receiving a frame. Reset the timer after
         * every character received. If more than the maximum possible
         * number of bytes in a modbus frame is received the frame is
         * ignored.
         */
    case STATE_M_RX_RCV:
        #if HDLC == 0
        if( usMasterRcvBufferPos < MB_SER_PDU_SIZE_MAX )
        {
            ucMasterRTURcvBuf[usMasterRcvBufferPos++] = ucByte;
        }
        else
        {
            eRcvState = STATE_M_RX_ERROR;
        }
        vMBMasterPortTimersT35Enable();
        break;
        #else
        vMBMasterPortTimersT35Enable( );

        if( usMasterRcvBufferPos < MB_SER_PDU_SIZE_MAX )
        {
            ucMasterRTURcvBuf[usMasterRcvBufferPos] = ucByte;
            for (uint8_t i = 7; i >= 0; i--)
            {
                if(ucMasterRTURcvBuf[usMasterRcvBufferPos] & (1 << i))
                    HDLC_Count++;
                else 
                    HDLC_Count = 0;
                if (HDLC_Count >= 6)
                {
                    xTaskNeedSwitch = xMBMasterPortEventPost(EV_MASTER_FRAME_RECEIVED);
                    vMBMasterPortTimersDisable(); 
                    eRcvState = STATE_M_RX_IDLE;             
                }  
            }
            usMasterRcvBufferPos++;          
        }
        else
        {
            eRcvState = STATE_M_RX_ERROR;
        }
        break;        
        #endif
    }
    return xTaskNeedSwitch;
}

uint8_t
xMBMasterRTUTransmitFSM( void )
{
    uint8_t            xNeedPoll = FALSE;

    switch ( eSndState )
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_M_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBMasterPortSerialEnable( TRUE, FALSE );
        break;

    case STATE_M_TX_XMIT:
        /* check if we are finished. */
        if( usMasterSndBufferCount != 0 )
        {
            xMBMasterPortSerialPutByte( ( uint8_t )*pucMasterSndBufferCur );
            pucMasterSndBufferCur++;  /* next byte in sendbuffer. */
            usMasterSndBufferCount--;
        }
        else
        {
            xFrameIsBroadcast = ( ucMasterRTUSndBuf[MB_SER_PDU_ADDR_OFF] == MB_ADDRESS_BROADCAST ) ? TRUE : FALSE;
            /* Disable transmitter. This prevents another transmit buffer
             * empty interrupt. */
            eSndState = STATE_M_TX_XFWR;
            vMBMasterPortSerialEnable( TRUE, FALSE );
            /* If the frame is broadcast ,master will enable timer of convert delay,
             * else master will enable timer of respond timeout. */
            if ( xFrameIsBroadcast == TRUE )
            {
                vMBMasterPortTimersConvertDelayEnable( );
            }
            else
            {
                vMBMasterPortTimersRespondTimeoutEnable( );
            }
        }
        break;

    default:
        break;
    }

    return xNeedPoll;
}

uint8_t
xMBMasterRTUTimerExpired(void)
{
    uint8_t xNeedPoll = FALSE;

    #if HDLC == 0
    switch (eRcvState)
    {
        /* Timer t35 expired. Startup phase is finished. */
    case STATE_M_RX_INIT:
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_READY);
        break;

        /* A frame was received and t35 expired. Notify the listener that
         * a new frame was received. */
    case STATE_M_RX_RCV:
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_FRAME_RECEIVED);
        break;

        /* An error occured while receiving the frame. */
    case STATE_M_RX_ERROR:
        vMBMasterSetErrorType(EV_ERROR_RECEIVE_DATA);
        xNeedPoll = xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
        break;

        /* Function called in an illegal state. */
    default:
        break;
    }
    #else 
    if (eRcvState == STATE_M_RX_INIT)
    {
        xNeedPoll = xMBMasterPortEventPost(EV_MASTER_READY);
    }
    #endif

    eRcvState = STATE_M_RX_IDLE;

    switch (eSndState)
    {
        /* A frame was send finish and convert delay or respond timeout expired.
         * If the frame is broadcast,The master will idle,and if the frame is not
         * broadcast.Notify the listener process error.*/
    case STATE_M_TX_XFWR:
        if ( xFrameIsBroadcast == FALSE ) {
            vMBMasterSetErrorType(EV_ERROR_RESPOND_TIMEOUT);
            xNeedPoll = xMBMasterPortEventPost(EV_MASTER_ERROR_PROCESS);
        }
        break;
        /* Function called in an illegal state. */
    default:
        break;
    }
    eSndState = STATE_M_TX_IDLE;

    vMBMasterPortTimersDisable( );
    /* If timer mode is convert delay, the master event then turns EV_MASTER_EXECUTE status. */
    if (eMasterCurTimerMode == MB_TMODE_CONVERT_DELAY) {
        xNeedPoll = xMBMasterPortEventPost( EV_MASTER_EXECUTE );
    }

    return xNeedPoll;
}

/* Get Modbus Master send RTU's buffer address pointer.*/
void vMBMasterGetRTUSndBuf( uint8_t ** pucFrame )
{
    *pucFrame = ( uint8_t * ) ucMasterRTUSndBuf;
}

/* Get Modbus Master send PDU's buffer address pointer.*/
void vMBMasterGetPDUSndBuf( uint8_t ** pucFrame )
{
    *pucFrame = ( uint8_t * ) &ucMasterRTUSndBuf[MB_SER_PDU_PDU_OFF];
}

/* Set Modbus Master send PDU's buffer length.*/
void vMBMasterSetPDUSndLength( uint16_t SendPDULength )
{
    usMasterSendPDULength = SendPDULength;
}

/* Get Modbus Master send PDU's buffer length.*/
uint16_t usMBMasterGetPDUSndLength( void )
{
    return usMasterSendPDULength;
}

/* Set Modbus Master current timer mode.*/
void vMBMasterSetCurTimerMode( eMBMasterTimerMode eMBTimerMode )
{
    eMasterCurTimerMode = eMBTimerMode;
}

/* The master request is broadcast? */
uint8_t xMBMasterRequestIsBroadcast( void ){
    return xFrameIsBroadcast;
}
#endif

