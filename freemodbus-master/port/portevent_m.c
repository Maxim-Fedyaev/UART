
#include "mbconfig.h"

#if MB_MASTER_RTU_ENABLED > 0
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_m.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Defines ------------------------------------------*/
/* ----------------------- Variables ----------------------------------------*/
static eMBMasterEventType eQueuedEvent;
static uint8_t xEventInQueue;
/* ----------------------- Start implementation -----------------------------*/
uint8_t
xMBMasterPortEventInit( void )
{
  xEventInQueue = FALSE;
  return TRUE;
}

uint8_t
xMBMasterPortEventPost( eMBMasterEventType eEvent )
{
  xEventInQueue = TRUE;
  eQueuedEvent = eEvent;
  return TRUE;
}

uint8_t
xMBMasterPortEventGet( eMBMasterEventType * eEvent )
{
  uint8_t xEventHappened = FALSE;
  if(xEventInQueue)
  {
    *eEvent = eQueuedEvent;
    xEventInQueue = FALSE;
    xEventHappened = TRUE;
  }
  return xEventHappened;
}
/**
 * This function is initialize the OS resource for modbus master.
 * Note:The resource is define by OS.If you not use OS this function can be empty.
 *
 */
void vMBMasterOsResInit( void )
{

}

/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
uint8_t xMBMasterRunResTake( int32_t lTimeOut )
{
    /*If waiting time is -1 .It will wait forever */
    return TRUE;
}

/**
 * This function is release Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be empty.
 *
 */
void vMBMasterRunResRelease( void )
{

}

/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(uint8_t ucDestAddress, const uint8_t* pucPDUData,
        uint16_t ucPDULength) {
  
  xMBMasterPortEventPost(EV_MASTER_ERROR_RESPOND_TIMEOUT);

}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(uint8_t ucDestAddress, const uint8_t* pucPDUData,
        uint16_t ucPDULength) {

    xMBMasterPortEventPost(EV_MASTER_ERROR_RECEIVE_DATA);

}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(uint8_t ucDestAddress, const uint8_t* pucPDUData,
        uint16_t ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_ERROR_EXECUTE_FUNCTION);

    /* You can add your code under here. */

}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestScuuess( void ) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xMBMasterPortEventPost(EV_MASTER_PROCESS_SUCCESS);

    /* You can add your code under here. */

}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void ) {
  eMBMasterReqErrCode eErrStatus = MB_MRE_NO_ERR;
  switch (eQueuedEvent)
  {
    case EV_MASTER_PROCESS_SUCCESS:
      break;
    case EV_MASTER_ERROR_RESPOND_TIMEOUT:
      eErrStatus = MB_MRE_TIMEDOUT;
      break;
    case EV_MASTER_ERROR_RECEIVE_DATA:
      eErrStatus = MB_MRE_REV_DATA;
      break;
    case EV_MASTER_ERROR_EXECUTE_FUNCTION:
      eErrStatus = MB_MRE_EXE_FUN;
      break;
    default:
      break;
  }
  return eErrStatus;
}
#endif
