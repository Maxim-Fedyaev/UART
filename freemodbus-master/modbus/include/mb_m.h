
#ifndef _MB_M_H
#define _MB_M_H

#include "port.h"
#include "mb.h"
#include "mbport.h"
#include "mbproto.h"


/*! \defgroup modbus Modbus
 * \code #include "mb_m.h" \endcode
 *
 * This module defines the interface for the application. It contains
 * the basic functions and types required to use the Modbus Master protocol stack.
 * A typical application will want to call eMBMasterInit() first. If the device
 * is ready to answer network requests it must then call eMBEnable() to activate
 * the protocol stack. In the main loop the function eMBMasterPoll() must be called
 * periodically. The time interval between pooling depends on the configured
 * Modbus timeout. If an RTOS is available a separate task should be created
 * and the task should always call the function eMBMasterPoll().
 *
 * \code
 * // Initialize protocol stack in RTU mode for a Master
 * eMBMasterInit( MB_RTU, 38400, MB_PAR_EVEN );
 * // Enable the Modbus Protocol Stack.
 * eMBMasterEnable(  );
 * for( ;; )
 * {
 *     // Call the main polling loop of the Modbus Master protocol stack.
 *     eMBMasterPoll(  );
 *     ...
 * }
 * \endcode
 */

/* ----------------------- Defines ------------------------------------------*/

/*! \ingroup modbus
 * \brief Use the default Modbus Master TCP port (502)
 */
#define MB_MASTER_TCP_PORT_USE_DEFAULT 0

/* ----------------------- Type definitions ---------------------------------*/
/*! \ingroup modbus
 * \brief Errorcodes used by all function in the Master request.
 */
typedef enum
{
    MB_MRE_NO_ERR,                  /*!< no error. */
    MB_MRE_NO_REG,                  /*!< illegal register address. */
    MB_MRE_ILL_ARG,                 /*!< illegal argument. */
    MB_MRE_REV_DATA,                /*!< receive data error. */
    MB_MRE_TIMEDOUT,                /*!< timeout error occurred. */
    MB_MRE_MASTER_BUSY,             /*!< master is busy now. */
    MB_MRE_EXE_FUN                  /*!< execute function error. */
} eMBMasterReqErrCode;
/*! \ingroup modbus
 *  \brief TimerMode is Master 3 kind of Timer modes.
 */
typedef enum
{
    MB_TMODE_T35,                   /*!< Master receive frame T3.5 timeout. */
    MB_TMODE_RESPOND_TIMEOUT,       /*!< Master wait respond for slave. */
    MB_TMODE_CONVERT_DELAY          /*!< Master sent broadcast ,then delay sometime.*/
}eMBMasterTimerMode;

/* ----------------------- Function prototypes ------------------------------*/
/*! \ingroup modbus
 * \brief Initialize the Modbus Master protocol stack.
 *
 * This functions initializes the ASCII or RTU module and calls the
 * init functions of the porting layer to prepare the hardware. Please
 * note that the receiver is still disabled and no Modbus frames are
 * processed until eMBMasterEnable( ) has been called.
 *
 * \param eMode If ASCII or RTU mode should be used.
 * \param ucPort The port to use. E.g. 1 for COM1 on windows. This value
 *   is platform dependent and some ports simply choose to ignore it.
 * \param ulBaudRate The baudrate. E.g. 19200. Supported baudrates depend
 *   on the porting layer.
 * \param eParity Parity used for serial transmission.
 *
 * \return If no error occurs the function returns eMBErrorCode::MB_ENOERR.
 *   The protocol is then in the disabled state and ready for activation
 *   by calling eMBMasterEnable( ). Otherwise one of the following error codes
 *   is returned:
 *    - eMBErrorCode::MB_EPORTERR IF the porting layer returned an error.
 */
eMBErrorCode    eMBMasterInit( uint8_t ucPort,
                         uint32_t ulBaudRate, eMBParity eParity );

/*! \ingroup modbus
 * \brief Initialize the Modbus Master protocol stack for Modbus TCP.
 *
 * This function initializes the Modbus TCP Module. Please note that
 * frame processing is still disabled until eMBEnable( ) is called.
 *
 * \param usTCPPort The TCP port to listen on.
 * \return If the protocol stack has been initialized correctly the function
 *   returns eMBErrorCode::MB_ENOERR. Otherwise one of the following error
 *   codes is returned:
 *    - eMBErrorCode::MB_EINVAL If the slave address was not valid. Valid
 *        slave addresses are in the range 1 - 247.
 *    - eMBErrorCode::MB_EPORTERR IF the porting layer returned an error.
 */
eMBErrorCode    eMBMasterTCPInit( uint16_t usTCPPort );

/*! \ingroup modbus
 * \brief Release resources used by the protocol stack.
 *
 * This function disables the Modbus Master protocol stack and release all
 * hardware resources. It must only be called when the protocol stack
 * is disabled.
 *
 * \note Note all ports implement this function. A port which wants to
 *   get an callback must define the macro MB_PORT_HAS_CLOSE to 1.
 *
 * \return If the resources where released it return eMBErrorCode::MB_ENOERR.
 *   If the protocol stack is not in the disabled state it returns
 *   eMBErrorCode::MB_EILLSTATE.
 */
eMBErrorCode    eMBMasterClose( void );

/*! \ingroup modbus
 * \brief Enable the Modbus Master protocol stack.
 *
 * This function enables processing of Modbus Master frames. Enabling the protocol
 * stack is only possible if it is in the disabled state.
 *
 * \return If the protocol stack is now in the state enabled it returns
 *   eMBErrorCode::MB_ENOERR. If it was not in the disabled state it
 *   return eMBErrorCode::MB_EILLSTATE.
 */
eMBErrorCode    eMBMasterEnable( void );

/*! \ingroup modbus
 * \brief Disable the Modbus Master protocol stack.
 *
 * This function disables processing of Modbus frames.
 *
 * \return If the protocol stack has been disabled it returns
 *  eMBErrorCode::MB_ENOERR. If it was not in the enabled state it returns
 *  eMBErrorCode::MB_EILLSTATE.
 */
eMBErrorCode    eMBMasterDisable( void );

/*! \ingroup modbus
 * \brief Check the Modbus Master protocol stack has established or not.
 *
 * This function must be called and check the return value before calling
 * any other functions.
 *
 * \return If the protocol stack has been established or not
 *  TRUE.  the protocol stack has established
 *  FALSE. the protocol stack hasn't established
 */
uint8_t            eMBMasterIsEstablished( void );

/*! \ingroup modbus
 * \brief The main pooling loop of the Modbus Master protocol stack.
 *
 * This function must be called periodically. The timer interval required
 * is given by the application dependent Modbus slave timeout. Internally the
 * function calls xMBMasterPortEventGet() and waits for an event from the receiver or
 * transmitter state machines.
 *
 * \return If the protocol stack is not in the enabled state the function
 *   returns eMBErrorCode::MB_EILLSTATE. Otherwise it returns
 *   eMBErrorCode::MB_ENOERR.
 */
eMBErrorCode    eMBMasterPoll( void );

/*! \ingroup modbus
 * \brief Registers a callback handler for a given function code.
 *
 * This function registers a new callback handler for a given function code.
 * The callback handler supplied is responsible for interpreting the Modbus PDU and
 * the creation of an appropriate response. In case of an error it should return
 * one of the possible Modbus exceptions which results in a Modbus exception frame
 * sent by the protocol stack.
 *
 * \param ucFunctionCode The Modbus function code for which this handler should
 *   be registers. Valid function codes are in the range 1 to 127.
 * \param pxHandler The function handler which should be called in case
 *   such a frame is received. If \c NULL a previously registered function handler
 *   for this function code is removed.
 *
 * \return eMBErrorCode::MB_ENOERR if the handler has been installed. If no
 *   more resources are available it returns eMBErrorCode::MB_ENORES. In this
 *   case the values in mbconfig.h should be adjusted. If the argument was not
 *   valid it returns eMBErrorCode::MB_EINVAL.
 */
eMBErrorCode    eMBMasterRegisterCB( uint8_t ucFunctionCode,
                               pxMBFunctionHandler pxHandler );

/* ----------------------- Callback -----------------------------------------*/

/*! \defgroup modbus_master registers Modbus Registers
 * \code #include "mb_m.h" \endcode
 * The protocol stack does not internally allocate any memory for the
 * registers. This makes the protocol stack very small and also usable on
 * low end targets. In addition the values don't have to be in the memory
 * and could for example be stored in a flash.<br>
 * Whenever the protocol stack requires a value it calls one of the callback
 * function with the register address and the number of registers to read
 * as an argument. The application should then read the actual register values
 * (for example the ADC voltage) and should store the result in the supplied
 * buffer.<br>
 * If the protocol stack wants to update a register value because a write
 * register function was received a buffer with the new register values is
 * passed to the callback function. The function should then use these values
 * to update the application register values.
 */

/*! \ingroup modbus_registers
 * \brief Callback function used if the value of a <em>Input Register</em>
 *   is required by the protocol stack. The starting register address is given
 *   by \c usAddress and the last register is given by <tt>usAddress +
 *   usNRegs - 1</tt>.
 *
 * \param pucRegBuffer A buffer where the callback function should write
 *   the current value of the modbus registers to.
 * \param usAddress The starting address of the register. Input registers
 *   are in the range 1 - 65535.
 * \param usNRegs Number of registers the callback function must supply.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMBErrorCode eMBMasterRegInputCB( uint8_t * pucRegBuffer, uint16_t usAddress,
        uint16_t usNRegs );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Holding Register</em> value is
 *   read or written by the protocol stack. The starting register address
 *   is given by \c usAddress and the last register is given by
 *   <tt>usAddress + usNRegs - 1</tt>.
 *
 * \param pucRegBuffer If the application registers values should be updated the
 *   buffer points to the new registers values. If the protocol stack needs
 *   to now the current values the callback function should write them into
 *   this buffer.
 * \param usAddress The starting address of the register.
 * \param usNRegs Number of registers to read or write.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application register
 *   values should be updated from the values in the buffer. For example
 *   this would be the case when the Modbus master has issued an
 *   <b>WRITE SINGLE REGISTER</b> command.
 *   If the value eMBRegisterMode::MB_REG_READ the application should copy
 *   the current values into the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMBErrorCode eMBMasterRegHoldingCB( uint8_t * pucRegBuffer, uint16_t usAddress,
        uint16_t usNRegs, eMBRegisterMode eMode );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Coil Register</em> value is
 *   read or written by the protocol stack. If you are going to use
 *   this function you might use the functions xMBUtilSetBits(  ) and
 *   xMBUtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The bits are packed in bytes where the first coil
 *   starting at address \c usAddress is stored in the LSB of the
 *   first byte in the buffer <code>pucRegBuffer</code>.
 *   If the buffer should be written by the callback function unused
 *   coil values (I.e. if not a multiple of eight coils is used) should be set
 *   to zero.
 * \param usAddress The first coil number.
 * \param usNCoils Number of coil values requested.
 * \param eMode If eMBRegisterMode::MB_REG_WRITE the application values should
 *   be updated from the values supplied in the buffer \c pucRegBuffer.
 *   If eMBRegisterMode::MB_REG_READ the application should store the current
 *   values in the buffer \c pucRegBuffer.
 *
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMBErrorCode eMBMasterRegCoilsCB( uint8_t * pucRegBuffer, uint16_t usAddress,
        uint16_t usNCoils, eMBRegisterMode eMode );

/*! \ingroup modbus_registers
 * \brief Callback function used if a <em>Input Discrete Register</em> value is
 *   read by the protocol stack.
 *
 * If you are going to use his function you might use the functions
 * xMBUtilSetBits(  ) and xMBUtilGetBits(  ) for working with bitfields.
 *
 * \param pucRegBuffer The buffer should be updated with the current
 *   coil values. The first discrete input starting at \c usAddress must be
 *   stored at the LSB of the first byte in the buffer. If the requested number
 *   is not a multiple of eight the remaining bits should be set to zero.
 * \param usAddress The starting address of the first discrete input.
 * \param usNDiscrete Number of discrete input values.
 * \return The function must return one of the following error codes:
 *   - eMBErrorCode::MB_ENOERR If no error occurred. In this case a normal
 *       Modbus response is sent.
 *   - eMBErrorCode::MB_ENOREG If the application does not map an coils
 *       within the requested address range. In this case a
 *       <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
 */
eMBErrorCode eMBMasterRegDiscreteCB( uint8_t * pucRegBuffer, uint16_t usAddress,
        uint16_t usNDiscrete );

/*! \ingroup modbus
 *\brief These Modbus functions are called for user when Modbus run in Master Mode.
 */
eMBMasterReqErrCode
eMBMasterReqReadInputRegister( uint8_t ucSndAddr, uint16_t usRegAddr, uint16_t usNRegs, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteHoldingRegister( uint8_t ucSndAddr, uint16_t usRegAddr, uint16_t usRegData, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteMultipleHoldingRegister( uint8_t ucSndAddr, uint16_t usRegAddr,
        uint16_t usNRegs, uint16_t * pusDataBuffer, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadHoldingRegister( uint8_t ucSndAddr, uint16_t usRegAddr, uint16_t usNRegs, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadWriteMultipleHoldingRegister( uint8_t ucSndAddr,
        uint16_t usReadRegAddr, uint16_t usNReadRegs, uint16_t * pusDataBuffer,
        uint16_t usWriteRegAddr, uint16_t usNWriteRegs, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadCoils( uint8_t ucSndAddr, uint16_t usCoilAddr, uint16_t usNCoils, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteCoil( uint8_t ucSndAddr, uint16_t usCoilAddr, uint16_t usCoilData, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqWriteMultipleCoils( uint8_t ucSndAddr,
        uint16_t usCoilAddr, uint16_t usNCoils, uint8_t * pucDataBuffer, int32_t lTimeOut );
eMBMasterReqErrCode
eMBMasterReqReadDiscreteInputs( uint8_t ucSndAddr, uint16_t usDiscreteAddr, uint16_t usNDiscreteIn, int32_t lTimeOut );

eMBException
eMBMasterFuncReportSlaveID( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncReadInputRegister( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncReadHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncWriteHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncWriteMultipleHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncReadCoils( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncWriteCoil( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncWriteMultipleCoils( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncReadDiscreteInputs( uint8_t * pucFrame, uint16_t * usLen );
eMBException
eMBMasterFuncReadWriteMultipleHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );

/*\ingroup modbus
 *\brief These functions are interface for Modbus Master
 */
void vMBMasterGetPDUSndBuf( uint8_t ** pucFrame );
uint8_t ucMBMasterGetDestAddress( void );
void vMBMasterSetDestAddress( uint8_t Address );
uint8_t xMBMasterGetCBRunInMasterMode( void );
void vMBMasterSetCBRunInMasterMode( uint8_t IsMasterMode );
uint16_t usMBMasterGetPDUSndLength( void );
void vMBMasterSetPDUSndLength( uint16_t SendPDULength );
void vMBMasterSetCurTimerMode( eMBMasterTimerMode eMBTimerMode );
uint8_t xMBMasterRequestIsBroadcast( void );
eMBMasterErrorEventType eMBMasterGetErrorType( void );
void vMBMasterSetErrorType( eMBMasterErrorEventType errorType );
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void );

/* ----------------------- Callback -----------------------------------------*/

#endif
