
#ifndef _PORT_H
#define _PORT_H

#include "mbconfig.h"
#include <stdint.h>

#define MB_SlaveAddress 5 // Адрес устройства

#define KUZNECHIK 0 // Шифрование вкл/выкл
#define HDLC 0 // бит-стаффинг

#define PAD_MODE_1 0x01 // Дополнение нулями до размера полного блока
#define PAD_MODE_2 0x02 // Дополнение 0x80 и нулями до размера полного блока и заполнение следующего блока нулями
#define PAD_MODE_3 0x03 // Дополнение 0x80 и нулями до размера полного блока

#define ENTER_CRITICAL_SECTION()    EnterCriticalSection()
#define EXIT_CRITICAL_SECTION()     ExitCriticalSection()

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

void EnterCriticalSection(void);
void ExitCriticalSection(void);

#endif
