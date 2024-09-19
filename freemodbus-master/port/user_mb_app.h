#ifndef    USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START                    10000
#define S_DISCRETE_INPUT_NDISCRETES               4
#define S_COIL_START                              20000
#define S_COIL_NCOILS                             130
#define S_REG_INPUT_START                         30000
#define S_REG_INPUT_NREGS                         4
#define S_REG_HOLDING_START                       40000
#define S_REG_HOLDING_NREGS                       130

/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START                    10000
#define M_DISCRETE_INPUT_NDISCRETES               4
#define M_COIL_START                              20000
#define M_COIL_NCOILS                             130
#define M_REG_INPUT_START                         30000
#define M_REG_INPUT_NREGS                         4
#define M_REG_HOLDING_START                       40000
#define M_REG_HOLDING_NREGS                       130

#endif