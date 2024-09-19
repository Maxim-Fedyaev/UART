
#ifndef _MB_FUNC_H
#define _MB_FUNC_H

#if MB_FUNC_OTHER_REP_SLAVEID_BUF > 0
eMBException    eMBFuncReportSlaveID( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_READ_INPUT_ENABLED > 0
eMBException    eMBFuncReadInputRegister( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_READ_HOLDING_ENABLED > 0
eMBException    eMBFuncReadHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
eMBException    eMBFuncWriteHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
eMBException    eMBFuncWriteMultipleHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_READ_COILS_ENABLED > 0
eMBException    eMBFuncReadCoils( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_WRITE_COIL_ENABLED > 0
eMBException    eMBFuncWriteCoil( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
eMBException    eMBFuncWriteMultipleCoils( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
eMBException    eMBFuncReadDiscreteInputs( uint8_t * pucFrame, uint16_t * usLen );
#endif

#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
eMBException    eMBFuncReadWriteMultipleHoldingRegister( uint8_t * pucFrame, uint16_t * usLen );
#endif

#endif
