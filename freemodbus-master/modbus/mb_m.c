
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/

#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

#include "mbport.h"

#include "mbrtu.h"


#if MB_MASTER_RTU_ENABLED > 0

#ifndef MB_PORT_HAS_CLOSE
#define MB_PORT_HAS_CLOSE 0
#endif

/* ----------------------- Static variables ---------------------------------*/

static uint8_t    ucMBMasterDestAddress;
static uint8_t     xMBRunInMasterMode = FALSE;
static eMBMasterErrorEventType eMBMasterCurErrorType;

static enum
{
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_NOT_INITIALIZED,
    STATE_ESTABLISHED,
} eMBState = STATE_NOT_INITIALIZED;

/* Functions pointer which are initialized in eMBInit( ). Depending on the
 * mode (RTU or ASCII) the are set to the correct implementations.
 * Using for Modbus Master,Add by Armink 20130813
 */
static peMBFrameSend peMBMasterFrameSendCur;
static pvMBFrameStart pvMBMasterFrameStartCur;
static pvMBFrameStop pvMBMasterFrameStopCur;
static peMBFrameReceive peMBMasterFrameReceiveCur;
static pvMBFrameClose pvMBMasterFrameCloseCur;

/* Callback functions required by the porting layer. They are called when
 * an external event has happend which includes a timeout or the reception
 * or transmission of a character.
 * Using for Modbus Master,Add by Armink 20130813
 */
uint8_t( *pxMBMasterFrameCBByteReceived ) ( void );
uint8_t( *pxMBMasterFrameCBTransmitterEmpty ) ( void );
uint8_t( *pxMBMasterPortCBTimerExpired ) ( void );

uint8_t( *pxMBMasterFrameCBReceiveFSMCur ) ( void );
uint8_t( *pxMBMasterFrameCBTransmitFSMCur ) ( void );

/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */
static xMBFunctionHandler xMasterFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
    //TODO Add Master function define
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBMasterFuncReadInputRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBMasterFuncReadHoldingRegister},
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBMasterFuncWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBMasterFuncWriteHoldingRegister},
#endif
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBMasterFuncReadWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_READ_COILS_ENABLED > 0
    {MB_FUNC_READ_COILS, eMBMasterFuncReadCoils},
#endif
#if MB_FUNC_WRITE_COIL_ENABLED > 0
    {MB_FUNC_WRITE_SINGLE_COIL, eMBMasterFuncWriteCoil},
#endif
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_COILS, eMBMasterFuncWriteMultipleCoils},
#endif
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
    {MB_FUNC_READ_DISCRETE_INPUTS, eMBMasterFuncReadDiscreteInputs},
#endif
};

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBMasterInit( uint8_t ucPort, uint32_t ulBaudRate, eMBParity eParity )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    pvMBMasterFrameStartCur = eMBMasterRTUStart;
    pvMBMasterFrameStopCur = eMBMasterRTUStop;
    peMBMasterFrameSendCur = eMBMasterRTUSend;
    peMBMasterFrameReceiveCur = eMBMasterRTUReceive;
    pvMBMasterFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBMasterPortClose : NULL;
    pxMBMasterFrameCBByteReceived = xMBMasterRTUReceiveFSM;
    pxMBMasterFrameCBTransmitterEmpty = xMBMasterRTUTransmitFSM;
    pxMBMasterPortCBTimerExpired = xMBMasterRTUTimerExpired;

    eStatus = eMBMasterRTUInit(ucPort, ulBaudRate, eParity);

    if (eStatus == MB_ENOERR)
    {
        if (!xMBMasterPortEventInit())
        {
            /* port dependent event module initalization failed. */
            eStatus = MB_EPORTERR;
        }
        else
        {
            eMBState = STATE_DISABLED;
        }
        /* initialize the OS resource for modbus master. */
        vMBMasterOsResInit();
    }
    return eStatus;
}

eMBErrorCode
eMBMasterClose( void )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        if( pvMBMasterFrameCloseCur != NULL )
        {
            pvMBMasterFrameCloseCur(  );
        }
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

eMBErrorCode
eMBMasterEnable( void )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        /* Activate the protocol stack. */
        pvMBMasterFrameStartCur(  );
        eMBState = STATE_ENABLED;
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }

    xMBMasterPortEventPost( EV_MASTER_FRAME_RECEIVED );

    return eStatus;
}

eMBErrorCode
eMBMasterDisable( void )
{
    eMBErrorCode    eStatus;

    if(( eMBState == STATE_ENABLED ) || ( eMBState == STATE_ESTABLISHED))
    {
        pvMBMasterFrameStopCur(  );
        eMBState = STATE_DISABLED;
        eStatus = MB_ENOERR;
    }
    else if( eMBState == STATE_DISABLED )
    {
        eStatus = MB_ENOERR;
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

uint8_t
eMBMasterIsEstablished( void )
{
    if(eMBState == STATE_ESTABLISHED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


eMBErrorCode
eMBMasterPoll( void )
{
    static uint8_t   *ucMBFrame;
    static uint8_t    ucRcvAddress;
    static uint8_t    ucFunctionCode;
    static uint16_t   usLength;
    static eMBException eException;

    int             i , j;
    eMBErrorCode    eStatus = MB_ENOERR;
    eMBMasterEventType    eEvent;
    eMBMasterErrorEventType errorType;

    /* Check if the protocol stack is ready. */
    if(( eMBState != STATE_ENABLED ) && ( eMBState != STATE_ESTABLISHED))
    {
        return MB_EILLSTATE;
    }

    /* Check if there is a event available. If not return control to caller.
     * Otherwise we will handle the event. */
    if( xMBMasterPortEventGet( &eEvent ) == TRUE )
    {
        switch ( eEvent )
        {
        case EV_MASTER_READY:
            eMBState = STATE_ESTABLISHED;
            break;

        case EV_MASTER_FRAME_RECEIVED:
            eStatus = peMBMasterFrameReceiveCur( &ucRcvAddress, &ucMBFrame, &usLength );
            /* Check if the frame is for us. If not ,send an error process event. */
            if ( ( eStatus == MB_ENOERR ) && ( ucRcvAddress == ucMBMasterGetDestAddress() ) )
            {
                ( void ) xMBMasterPortEventPost( EV_MASTER_EXECUTE );
            }
            else
            {
                vMBMasterSetErrorType(EV_ERROR_RECEIVE_DATA);
                ( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
            }
            break;

        case EV_MASTER_EXECUTE:
            ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
            eException = MB_EX_ILLEGAL_FUNCTION;
            /* If receive frame has exception .The receive function code highest bit is 1.*/
            if(ucFunctionCode >> 7) {
                eException = (eMBException)ucMBFrame[MB_PDU_DATA_OFF];
            }
            else
            {
                for (i = 0; i < MB_FUNC_HANDLERS_MAX; i++)
                {
                    /* No more function handlers registered. Abort. */
                    if (xMasterFuncHandlers[i].ucFunctionCode == 0) {
                        break;
                    }
                    else if (xMasterFuncHandlers[i].ucFunctionCode == ucFunctionCode) {
                        vMBMasterSetCBRunInMasterMode(TRUE);
                        /* If master request is broadcast,
                         * the master need execute function for all slave.
                         */
                        if ( xMBMasterRequestIsBroadcast() ) {
                            usLength = usMBMasterGetPDUSndLength();
                            for(j = 1; j <= MB_MASTER_TOTAL_SLAVE_NUM; j++){
                                vMBMasterSetDestAddress(j);
                                eException = xMasterFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
                            }
                        }
                        else {
                            eException = xMasterFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
                        }
                        vMBMasterSetCBRunInMasterMode(FALSE);
                        break;
                    }
                }
            }
            /* If master has exception ,Master will send error process.Otherwise the Master is idle.*/
            if (eException != MB_EX_NONE) {
                vMBMasterSetErrorType(EV_ERROR_EXECUTE_FUNCTION);
                ( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
            }
            else {
                vMBMasterCBRequestScuuess( );
                vMBMasterRunResRelease( );
            }
            break;

        case EV_MASTER_FRAME_SENT:
            /* Master is busy now. */
            vMBMasterGetPDUSndBuf( &ucMBFrame );
            eStatus = peMBMasterFrameSendCur( ucMBMasterGetDestAddress(), ucMBFrame, usMBMasterGetPDUSndLength() );
            break;

        case EV_MASTER_ERROR_PROCESS:
            /* Execute specified error process callback function. */
            errorType = eMBMasterGetErrorType();
            vMBMasterGetPDUSndBuf( &ucMBFrame );
            switch (errorType) {
            case EV_ERROR_RESPOND_TIMEOUT:
                vMBMasterErrorCBRespondTimeout(ucMBMasterGetDestAddress(),
                        ucMBFrame, usMBMasterGetPDUSndLength());
                break;
            case EV_ERROR_RECEIVE_DATA:
                vMBMasterErrorCBReceiveData(ucMBMasterGetDestAddress(),
                        ucMBFrame, usMBMasterGetPDUSndLength());
                break;
            case EV_ERROR_EXECUTE_FUNCTION:
                vMBMasterErrorCBExecuteFunction(ucMBMasterGetDestAddress(),
                        ucMBFrame, usMBMasterGetPDUSndLength());
                break;
            }
            vMBMasterRunResRelease();
            break;

        default:
            break;
        }

    }
    return MB_ENOERR;
}

/* Get whether the Modbus Master is run in master mode.*/
uint8_t xMBMasterGetCBRunInMasterMode( void )
{
    return xMBRunInMasterMode;
}
/* Set whether the Modbus Master is run in master mode.*/
void vMBMasterSetCBRunInMasterMode( uint8_t IsMasterMode )
{
    xMBRunInMasterMode = IsMasterMode;
}
/* Get Modbus Master send destination address. */
uint8_t ucMBMasterGetDestAddress( void )
{
    return ucMBMasterDestAddress;
}
/* Set Modbus Master send destination address. */
void vMBMasterSetDestAddress( uint8_t Address )
{
    ucMBMasterDestAddress = Address;
}
/* Get Modbus Master current error event type. */
eMBMasterErrorEventType eMBMasterGetErrorType( void )
{
    return eMBMasterCurErrorType;
}
/* Set Modbus Master current error event type. */
void vMBMasterSetErrorType( eMBMasterErrorEventType errorType )
{
    eMBMasterCurErrorType = errorType;
}



#endif
