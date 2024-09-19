#include "hdlcbitstaffing.h"
#include "Kuznechik.h"

/**
 * Функция операции, обратной бит-стаффингу
 *
 * @param HDLC_RXTX_buffer Массив входных данных
 * @param Length_HDLC_RXTX_buffer Длина массива входных данных
 * @param HDLC_buffer Массив выходных данных
 *
 * @return Код результата операции
 */
eMBErrorCode Res_bitstaffing(volatile uint8_t* HDLC_RXTX_buffer, volatile uint16_t Length_HDLC_RXTX_buffer, volatile uint8_t* HDLC_buffer)
{
    volatile char HDLC_delete_0 = 0;
    volatile char HDLC_CTR_1 = 0;
    char HDLC_buffer_bit = 7;  
    int HDLC_buffer_byte = 1;
    int HDLC_RXTX_buffer_byte = 1;

    if(HDLC_RXTX_buffer[0] != 0x7E)
        return MB_ERRORFRAME;
    HDLC_buffer[0] = 0x7E;
    while(HDLC_RXTX_buffer_byte < Length_HDLC_RXTX_buffer)
    {
        for(char HDLC_RXTX_buffer_bit = 7; HDLC_RXTX_buffer_bit >= 0; HDLC_RXTX_buffer_bit--)
        {
            HDLC_delete_0 = 0;
            if(HDLC_RXTX_buffer[HDLC_RXTX_buffer_byte] & (1<<HDLC_RXTX_buffer_bit))
                HDLC_CTR_1++;
            else if (HDLC_CTR_1 == 6)
            {
                HDLC_buffer[HDLC_buffer_byte] |=  (HDLC_RXTX_buffer[HDLC_RXTX_buffer_byte] & (1 << HDLC_RXTX_buffer_bit)) ? (1<<HDLC_buffer_bit) : 0;
                return MB_ENOERR;
            }
            else if (HDLC_CTR_1 > 6 && HDLC_CTR_1 < 15)
                return MB_ENDFAULT;                
            else if (HDLC_CTR_1 >= 15)
                return MB_SLEEP;
            else if (HDLC_CTR_1 == 5)
            {
                HDLC_delete_0 = 1;
                HDLC_CTR_1 = 0; 
            }               
            else          
                HDLC_CTR_1 = 0;
            if(HDLC_delete_0 != 1)
            {
                HDLC_buffer[HDLC_buffer_byte] |= (HDLC_RXTX_buffer[HDLC_RXTX_buffer_byte] & (1 << HDLC_RXTX_buffer_bit)) ? (1<<HDLC_buffer_bit) : 0;
                HDLC_buffer_bit--;
                if(HDLC_buffer_bit < 0)
                {
                    HDLC_buffer_bit = 7;
                    HDLC_buffer_byte++;
                }
            }
        }
        HDLC_RXTX_buffer_byte++;
    }
    return MB_ERRORFRAME;
}

/**
 * Функция операции бит-стаффинга
 *
 * @param HDLC_buffer Массив входных данных
 * @param Length_HDLC_buffer Длина массива входных данных
 * @param HDLC_RXTX_buffer Массив выходных данных
 *
 * @return Длину массива выходных данных
 */
uint16_t Trans_bitstaffing(volatile uint8_t* HDLC_buffer, volatile uint16_t Length_HDLC_buffer, volatile uint8_t* HDLC_RXTX_buffer)
{
    char HDLC_CTR_1 = 0;
    char HDLC_RXTX_buffer_bit = 7;  
    int HDLC_RXTX_buffer_byte = 1;

    HDLC_RXTX_buffer[0] = FLAG_FD;
    for(int HDLC_buffer_byte = 0; HDLC_buffer_byte < Length_HDLC_buffer; HDLC_buffer_byte++)
    {
        for(char HDLC_buffer_bit = 7; HDLC_buffer_bit >= 0; HDLC_buffer_bit--)
        {
            if(HDLC_buffer[HDLC_buffer_byte] & 1<<HDLC_buffer_bit)
                HDLC_CTR_1++;            
            else          
                HDLC_CTR_1 = 0;

            HDLC_RXTX_buffer[HDLC_RXTX_buffer_byte] |= (HDLC_buffer[HDLC_buffer_byte] & 1<<HDLC_buffer_bit) ? (1<<HDLC_RXTX_buffer_bit) : 0;
            HDLC_RXTX_buffer_bit--;
            if(HDLC_RXTX_buffer_bit < 0)
            {
                HDLC_RXTX_buffer_bit = 7;
                HDLC_RXTX_buffer_byte++;
            }
            if(HDLC_CTR_1 == 5)
            {
                HDLC_RXTX_buffer_bit--;
                if(HDLC_RXTX_buffer_bit < 0)
                {
                    HDLC_RXTX_buffer_bit = 7;
                    HDLC_RXTX_buffer_byte++;
                }
                HDLC_CTR_1 = 0;
            }
        }
    }
    for(char HDLC_buffer_bit = 7; HDLC_buffer_bit >= 0; HDLC_buffer_bit--)
    {
        HDLC_RXTX_buffer[HDLC_RXTX_buffer_byte] |= (FLAG_FD & 1<<HDLC_buffer_bit) ? (1<<HDLC_RXTX_buffer_bit) : 0;
        HDLC_RXTX_buffer_bit--;
        if(HDLC_RXTX_buffer_bit < 0 && HDLC_buffer_bit != 0)
        {
            HDLC_RXTX_buffer_bit = 7;
            HDLC_RXTX_buffer_byte++;
        }
    }
    return HDLC_RXTX_buffer_byte+1;    
}

/**
 * Операция декодирования (дебит-стаффинг+дешифрация)
 *
 * @param input_buffer Массив входных данных
 * @param length_input_buffer Длина массива входных данных
 * @param output_buffer Массив выходных данных
 * 
 * @return Код результата операции
 */
eMBErrorCode Decoder (volatile uint8_t* input_buffer, volatile uint16_t* indicator_length_input_buffer, volatile uint8_t* output_buffer)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    uint8_t HDLC_buffer[*indicator_length_input_buffer];
    eStatus = Res_bitstaffing(input_buffer, *indicator_length_input_buffer, HDLC_buffer);
    if (eStatus == MB_ENOERR)
    {
    #if KUZNECHIK
        uint16_t Length_HDLC_buffer = 0;    
        for (uint16_t i = 2; i < sizeof(HDLC_buffer)/sizeof(*HDLC_buffer); i++) 
        {
            if (HDLC_buffer[i] != FLAG_FD) 
                Length_HDLC_buffer++; 
            else 
                break;
        }    
        uint8_t Decrypt_buffer[Length_HDLC_buffer];
        for (uint16_t i = 0; i < Length_HDLC_buffer; i++) Decrypt_buffer[i] = HDLC_buffer[i+2];
        Decrypt(Decrypt_buffer, sizeof(Decrypt_buffer)/sizeof(*Decrypt_buffer));
        uint16_t i = 0, Length_output_buffer = 0;
        while (Decrypt_buffer[i] != 0x80 || Decrypt_buffer[i+1] != 0) { Length_output_buffer++; i++; }
        output_buffer[0] = HDLC_buffer[1];
        for (uint16_t i = 1; i < Length_output_buffer + 1; i++) output_buffer[i] = Decrypt_buffer[i-1];
        *indicator_length_input_buffer = Length_output_buffer + 1;                  
    #else 
        uint16_t Length_output_buffer = 0;
        for (uint16_t i = 1; i < sizeof(HDLC_buffer)/sizeof(*HDLC_buffer); i++) 
        {
            if (HDLC_buffer[i] != FLAG_FD) 
                Length_output_buffer++; 
            else 
                break;
        }
        for (uint16_t i = 0; i < Length_output_buffer; i++) output_buffer[i] = HDLC_buffer[i+1];  
        *indicator_length_input_buffer = Length_output_buffer;                       
    #endif
    }
        return eStatus;   
}
/**
 * Операция кодирования (шифрация+бит-стаффинг)
 *
 * @param input_buffer Массив входных данных
 * @param length_input_buffer Длина массива входных данных
 * @param output_buffer Массив выходных данных
 * 
 * @result Длину выходного массива
 */
uint16_t Coder (volatile uint8_t* input_buffer, volatile uint16_t length_input_buffer, volatile uint8_t* output_buffer)
{
    #if KUZNECHIK
        uint8_t uint8plain_text[length_input_buffer - 1 + get_size_pad(length_input_buffer - 1, PAD_MODE_3)];
        set_padding(uint8plain_text, input_buffer + 1, get_size_pad(length_input_buffer - 1, PAD_MODE_3), length_input_buffer - 1, PAD_MODE_3);
        Encrypt(uint8plain_text, sizeof(uint8plain_text)/sizeof(*uint8plain_text));
        uint8_t HDLC_buffer[sizeof(uint8plain_text)/sizeof(*uint8plain_text) + 1];
        HDLC_buffer[0] = input_buffer[0];
        for (uint16_t i = 1; i < sizeof(uint8plain_text)/sizeof(*uint8plain_text) + 1; i++) HDLC_buffer[i] = uint8plain_text[i - 1];
        uint16_t Length_output_buffer = sizeof(uint8plain_text)/sizeof(*uint8plain_text)*1.2+1+2;
        uint8_t HDLC_RXTX_buffer[Length_output_buffer];
        for (uint16_t i = 0; i < Length_output_buffer; i++) HDLC_RXTX_buffer[i] = 0; 
        Length_output_buffer = Trans_bitstaffing(HDLC_buffer, sizeof(uint8plain_text)/sizeof(*uint8plain_text) + 1, HDLC_RXTX_buffer);
        for(uint16_t i = 0; i < Length_output_buffer; i++) output_buffer[i] = HDLC_RXTX_buffer[i];                            
    #else 
        uint8_t HDLC_buffer[length_input_buffer];
        for (uint16_t i = 0; i < length_input_buffer; i++) HDLC_buffer[i] = input_buffer[i]; 
        uint16_t Length_output_buffer = length_input_buffer*1.2+1+2;
        uint8_t HDLC_RXTX_buffer[Length_output_buffer];
        for (uint16_t i = 0; i < Length_output_buffer; i++) HDLC_RXTX_buffer[i] = 0; 
        Length_output_buffer = Trans_bitstaffing(HDLC_buffer, length_input_buffer, HDLC_RXTX_buffer);
        for(uint16_t i = 0; i < Length_output_buffer; i++) output_buffer[i] = HDLC_RXTX_buffer[i];           
    #endif
    return Length_output_buffer;
}