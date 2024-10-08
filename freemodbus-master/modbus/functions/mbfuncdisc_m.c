
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"
#include "user_mb_app.h"
/* ----------------------- Defines ------------------------------------------*/
#define MB_PDU_REQ_READ_ADDR_OFF            ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_REQ_READ_DISCCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_REQ_READ_SIZE                ( 4 )
#define MB_PDU_FUNC_READ_DISCCNT_OFF        ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_READ_VALUES_OFF         ( MB_PDU_DATA_OFF + 1 )
#define MB_PDU_FUNC_READ_SIZE_MIN           ( 1 )

/* ----------------------- Static functions ---------------------------------*/
eMBException    prveMBError2Exception( eMBErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0

/**
 * This function will request read discrete inputs.
 *
 * @param ucSndAddr salve address
 * @param usDiscreteAddr discrete start address
 * @param usNDiscreteIn discrete total number
 * @param lTimeOut timeout (-1 will waiting forever)
 *
 * @return error code
 */
eMBMasterReqErrCode
eMBMasterReqReadDiscreteInputs( uint8_t ucSndAddr, uint16_t usDiscreteAddr, uint16_t usNDiscreteIn, int32_t lTimeOut )
{
    uint8_t                 *ucMBFrame;
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;

    if ( ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM ) eErrStatus = MB_MRE_ILL_ARG;
    else if ( xMBMasterRunResTake( lTimeOut ) == FALSE ) eErrStatus = MB_MRE_MASTER_BUSY;
    else
    {
        vMBMasterGetPDUSndBuf(&ucMBFrame);
        vMBMasterSetDestAddress(ucSndAddr);
        ucMBFrame[MB_PDU_FUNC_OFF]                 = MB_FUNC_READ_DISCRETE_INPUTS;
        ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF]        = usDiscreteAddr >> 8;
        ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1]    = usDiscreteAddr;
        ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF ]    = usNDiscreteIn >> 8;
        ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF + 1] = usNDiscreteIn;
        vMBMasterSetPDUSndLength( MB_PDU_SIZE_MIN + MB_PDU_REQ_READ_SIZE );
        ( void ) xMBMasterPortEventPost( EV_MASTER_FRAME_SENT );
        eErrStatus = eMBMasterWaitRequestFinish( );
    }
    return eErrStatus;
}

eMBException
eMBMasterFuncReadDiscreteInputs( uint8_t * pucFrame, uint16_t * usLen )
{
    uint16_t          usRegAddress;
    uint16_t          usDiscreteCnt;
    uint8_t           ucNBytes;
    uint8_t          *ucMBFrame;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    /* If this request is broadcast, and it's read mode. This request don't need execute. */
    if ( xMBMasterRequestIsBroadcast() )
    {
        eStatus = MB_EX_NONE;
    }
    else if( *usLen >= MB_PDU_SIZE_MIN + MB_PDU_FUNC_READ_SIZE_MIN )
    {
        vMBMasterGetPDUSndBuf(&ucMBFrame);
        usRegAddress = ( uint16_t )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF] << 8 );
        usRegAddress |= ( uint16_t )( ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1] );
        usRegAddress++;

        usDiscreteCnt = ( uint16_t )( ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF] << 8 );
        usDiscreteCnt |= ( uint16_t )( ucMBFrame[MB_PDU_REQ_READ_DISCCNT_OFF + 1] );

        /* Test if the quantity of coils is a multiple of 8. If not last
         * byte is only partially field with unused coils set to zero. */
        if( ( usDiscreteCnt & 0x0007 ) != 0 )
        {
            ucNBytes = ( uint8_t )( usDiscreteCnt / 8 + 1 );
        }
        else
        {
            ucNBytes = ( uint8_t )( usDiscreteCnt / 8 );
        }

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception.
         */
        if ((usDiscreteCnt >= 1) && ucNBytes == pucFrame[MB_PDU_FUNC_READ_DISCCNT_OFF])
        {
            /* Make callback to fill the buffer. */
            eRegStatus = eMBMasterRegDiscreteCB( &pucFrame[MB_PDU_FUNC_READ_VALUES_OFF], usRegAddress, usDiscreteCnt );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid read coil register request because the length
         * is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

#endif
#endif
