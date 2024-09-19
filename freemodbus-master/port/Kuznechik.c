#include <stdint.h>
#include <stdio.h>
#include "port.h"

#define  BLOCK_SIZE  16  // Размер блока
#define  KEY_LENGTH  32  // Длина массива с ключом
#define  SUB_LENGTH  (KEY_LENGTH/2)

uint8_t _subKeys[10][SUB_LENGTH];

int  pi[]=  {
            0xFC, 0xEE, 0xDD, 0x11, 0xCF, 0x6E, 0x31, 0x16, 	// 00..07
            0xFB, 0xC4, 0xFA, 0xDA, 0x23, 0xC5, 0x04, 0x4D, 	// 08..0F
            0xE9, 0x77, 0xF0, 0xDB, 0x93, 0x2E, 0x99, 0xBA, 	// 10..17
            0x17, 0x36, 0xF1, 0xBB, 0x14, 0xCD, 0x5F, 0xC1, 	// 18..1F
            0xF9, 0x18, 0x65, 0x5A, 0xE2, 0x5C, 0xEF, 0x21, 	// 20..27
            0x81, 0x1C, 0x3C, 0x42, 0x8B, 0x01, 0x8E, 0x4F, 	// 28..2F
            0x05, 0x84, 0x02, 0xAE, 0xE3, 0x6A, 0x8F, 0xA0, 	// 30..37
            0x06, 0x0B, 0xED, 0x98, 0x7F, 0xD4, 0xD3, 0x1F, 	// 38..3F
            0xEB, 0x34, 0x2C, 0x51, 0xEA, 0xC8, 0x48, 0xAB, 	// 40..47
            0xF2, 0x2A, 0x68, 0xA2, 0xFD, 0x3A, 0xCE, 0xCC, 	// 48..4F
            0xB5, 0x70, 0x0E, 0x56, 0x08, 0x0C, 0x76, 0x12, 	// 50..57
            0xBF, 0x72, 0x13, 0x47, 0x9C, 0xB7, 0x5D, 0x87, 	// 58..5F
            0x15, 0xA1, 0x96, 0x29, 0x10, 0x7B, 0x9A, 0xC7, 	// 60..67
            0xF3, 0x91, 0x78, 0x6F, 0x9D, 0x9E, 0xB2, 0xB1, 	// 68..6F
            0x32, 0x75, 0x19, 0x3D, 0xFF, 0x35, 0x8A, 0x7E, 	// 70..77
            0x6D, 0x54, 0xC6, 0x80, 0xC3, 0xBD, 0x0D, 0x57, 	// 78..7F
            0xDF, 0xF5, 0x24, 0xA9, 0x3E, 0xA8, 0x43, 0xC9, 	// 80..87
            0xD7, 0x79, 0xD6, 0xF6, 0x7C, 0x22, 0xB9, 0x03, 	// 88..8F
            0xE0, 0x0F, 0xEC, 0xDE, 0x7A, 0x94, 0xB0, 0xBC, 	// 90..97
            0xDC, 0xE8, 0x28, 0x50, 0x4E, 0x33, 0x0A, 0x4A, 	// 98..9F
            0xA7, 0x97, 0x60, 0x73, 0x1E, 0x00, 0x62, 0x44, 	// A0..A7
            0x1A, 0xB8, 0x38, 0x82, 0x64, 0x9F, 0x26, 0x41, 	// A8..AF
      		0xAD, 0x45, 0x46, 0x92, 0x27, 0x5E, 0x55, 0x2F, 	// B0..B7
            0x8C, 0xA3, 0xA5, 0x7D, 0x69, 0xD5, 0x95, 0x3B, 	// B8..BF
            0x07, 0x58, 0xB3, 0x40, 0x86, 0xAC, 0x1D, 0xF7, 	// C0..C7
            0x30, 0x37, 0x6B, 0xE4, 0x88, 0xD9, 0xE7, 0x89, 	// C8..CF
            0xE1, 0x1B, 0x83, 0x49, 0x4C, 0x3F, 0xF8, 0xFE, 	// D0..D7
            0x8D, 0x53, 0xAA, 0x90, 0xCA, 0xD8, 0x85, 0x61, 	// D8..DF
            0x20, 0x71, 0x67, 0xA4, 0x2D, 0x2B, 0x09, 0x5B, 	// E0..E7
            0xCB, 0x9B, 0x25, 0xD0, 0xBE, 0xE5, 0x6C, 0x52, 	// E8..EF
            0x59, 0xA6, 0x74, 0xD2, 0xE6, 0xF4, 0xB4, 0xC0, 	// F0..F7
            0xD1, 0x66, 0xAF, 0xC2, 0x39, 0x4B, 0x63, 0xB6, 	// F8..FF            
            };

int pi_inv[]={
            0xA5, 0x2D, 0x32, 0x8F, 0x0E, 0x30, 0x38, 0xC0, 	// 00..07
			0x54, 0xE6, 0x9E, 0x39, 0x55, 0x7E, 0x52, 0x91, 	// 08..0F
			0x64, 0x03, 0x57, 0x5A, 0x1C, 0x60, 0x07, 0x18, 	// 10..17
			0x21, 0x72, 0xA8, 0xD1, 0x29, 0xC6, 0xA4, 0x3F, 	// 18..1F
			0xE0, 0x27, 0x8D, 0x0C, 0x82, 0xEA, 0xAE, 0xB4, 	// 20..27
			0x9A, 0x63, 0x49, 0xE5, 0x42, 0xE4, 0x15, 0xB7, 	// 28..2F
			0xC8, 0x06, 0x70, 0x9D, 0x41, 0x75, 0x19, 0xC9, 	// 30..37
			0xAA, 0xFC, 0x4D, 0xBF, 0x2A, 0x73, 0x84, 0xD5, 	// 38..3F
			0xC3, 0xAF, 0x2B, 0x86, 0xA7, 0xB1, 0xB2, 0x5B, 	// 40..47
			0x46, 0xD3, 0x9F, 0xFD, 0xD4, 0x0F, 0x9C, 0x2F, 	// 48..4F
			0x9B, 0x43, 0xEF, 0xD9, 0x79, 0xB6, 0x53, 0x7F, 	// 50..57
			0xC1, 0xF0, 0x23, 0xE7, 0x25, 0x5E, 0xB5, 0x1E, 	// 58..5F
			0xA2, 0xDF, 0xA6, 0xFE, 0xAC, 0x22, 0xF9, 0xE2, 	// 60..67
			0x4A, 0xBC, 0x35, 0xCA, 0xEE, 0x78, 0x05, 0x6B, 	// 68..6F
			0x51, 0xE1, 0x59, 0xA3, 0xF2, 0x71, 0x56, 0x11, 	// 70..77
			0x6A, 0x89, 0x94, 0x65, 0x8C, 0xBB, 0x77, 0x3C, 	// 78..7F
			0x7B, 0x28, 0xAB, 0xD2, 0x31, 0xDE, 0xC4, 0x5F, 	// 80..87
			0xCC, 0xCF, 0x76, 0x2C, 0xB8, 0xD8, 0x2E, 0x36, 	// 88..8F
			0xDB, 0x69, 0xB3, 0x14, 0x95, 0xBE, 0x62, 0xA1, 	// 90..97
			0x3B, 0x16, 0x66, 0xE9, 0x5C, 0x6C, 0x6D, 0xAD, 	// 98..9F
			0x37, 0x61, 0x4B, 0xB9, 0xE3, 0xBA, 0xF1, 0xA0, 	// A0..A7
			0x85, 0x83, 0xDA, 0x47, 0xC5, 0xB0, 0x33, 0xFA, 	// A8..AF
			0x96, 0x6F, 0x6E, 0xC2, 0xF6, 0x50, 0xFF, 0x5D, 	// B0..B7
			0xA9, 0x8E, 0x17, 0x1B, 0x97, 0x7D, 0xEC, 0x58, 	// B8..BF
			0xF7, 0x1F, 0xFB, 0x7C, 0x09, 0x0D, 0x7A, 0x67, 	// C0..C7
			0x45, 0x87, 0xDC, 0xE8, 0x4F, 0x1D, 0x4E, 0x04, 	// C8..CF
			0xEB, 0xF8, 0xF3, 0x3E, 0x3D, 0xBD, 0x8A, 0x88, 	// D0..D7
			0xDD, 0xCD, 0x0B, 0x13, 0x98, 0x02, 0x93, 0x80, 	// D8..DF
			0x90, 0xD0, 0x24, 0x34, 0xCB, 0xED, 0xF4, 0xCE, 	// E0..E7
			0x99, 0x10, 0x44, 0x40, 0x92, 0x3A, 0x01, 0x26, 	// E8..EF
			0x12, 0x1A, 0x48, 0x68, 0xF5, 0x81, 0x8B, 0xC7, 	// F0..F7
			0xD6, 0x20, 0x0A, 0x08, 0x00, 0x4C, 0xD7, 0x74	 	// F8..FF            
            };

uint8_t _lFactors[] = {
            0x94, 0x20, 0x85, 0x10, 0xC2, 0xC0, 0x01, 0xFB,
            0x01, 0xC0, 0xC2, 0x10, 0x85, 0x20, 0x94, 0x01
                    };

uint8_t kuz_mul_gf256_slow(uint8_t a, uint8_t b)
{
    uint8_t p = 0;
    uint8_t counter;
    uint8_t hi_bit_set;
    for (counter = 0; counter < 8; counter++)
    {
        if ((b & 1) != 0)
            p ^= a;
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set != 0)
            a ^= 0xc3; /* x^8 + x^7 + x^6 + x + 1 */
        b >>= 1;
    }
    return p;
}

void S_Transformation(uint8_t* data, int data_length)
{
    for (int i = 0; i < data_length; i++)
    {
        data[i] = pi[data[i]];
    }
}

void S_Transformation_inverse(uint8_t* data, int data_length)
{
    for (int i = 0; i < data_length; i++)
    {
        data[i] = pi_inv[data[i]];
    }
}

void R_Transformation(uint8_t* data)
{
    uint8_t z = data[15];
    for (int i = 14; i >= 0; i--)
    {
        z ^= kuz_mul_gf256_slow(data[i], _lFactors[i]);
    }
    for (int i = 15; i > 0; i--)
    {
        data[i] = data[i - 1];
    }
    data[0] = z;

}

void R_Transformation_inverse(uint8_t* data)
{
    uint8_t z = data[0];
    for (int i = 0; i < 15; i++)
    {
        data[i] = data[i + 1];
        z ^= kuz_mul_gf256_slow(data[i], _lFactors[i]);
    }
    data[15] = z;
}

void L_Transformation(uint8_t* data)
{
    for (int i = 0; i < 16; i++)
    {
        R_Transformation(data);
    }
}

void L_Transformation_inverse(uint8_t* data)
{
    for (int i = 0; i < 16; i++)
    {
        R_Transformation_inverse(data);
    }
}

void X_Transformation(uint8_t* result, int result_length, uint8_t* data)
{
    for (int i = 0; i < result_length; i++)
    {
        result[i] ^= data[i];
    }
}

void F_Transformation(uint8_t* k, uint8_t* a1, uint8_t* a0)
{
    uint8_t temp[SUB_LENGTH];
    for (int j = 0; j < BLOCK_SIZE; j++) 
    { 
        temp[j] = k[j]; 
    }    
    X_Transformation(temp, BLOCK_SIZE, a1);
    S_Transformation(temp, BLOCK_SIZE);
    L_Transformation(temp);
    X_Transformation(temp, SUB_LENGTH, a0);
    for (int i = 0; i < SUB_LENGTH; i++) 
    { 
        a0[i] = a1[i]; 
    }
    for (int i = 0; i < SUB_LENGTH; i++) 
    { 
        a1[i] = temp[i]; 
    }
}

void C_Transformation(uint8_t*  c, int i)
{
    for (int i = 0; i < SUB_LENGTH; i++) 
    { 
        c[i] = 0; 
    }      
    c[15] = i;
    L_Transformation(c);
}

/**
 * Функция шифрования
 *
 * @param data массив данных
 * @param data_length длина массива
 *
 * Результат шифрования также в массиве data
 */
void Encrypt(uint8_t* data, int data_length)
{
    uint8_t block[BLOCK_SIZE];
    uint8_t temp[BLOCK_SIZE];
    for (int j = 0; j < data_length/BLOCK_SIZE; j++)
    {
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            block[i] = data[BLOCK_SIZE * j + i];
        }
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < BLOCK_SIZE; j++) 
            { 
                temp[j] = _subKeys[i][j]; 
            }    
            X_Transformation(temp, BLOCK_SIZE, block);
            S_Transformation(temp, BLOCK_SIZE);
            L_Transformation(temp);
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                block[j] = temp[j];
            }
        }
        X_Transformation(block, BLOCK_SIZE, _subKeys[9]);
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            data[BLOCK_SIZE * j + i] = block[i];
        }        
    }
}

/**
 * Функция дешифрования
 *
 * @param data массив данных
 * @param data_length длина массива
 *
 * Результат дешифрования также в массиве data
 */
void Decrypt(uint8_t* data, int data_length)
{
    uint8_t block[BLOCK_SIZE];
    for (int j = 0; j < data_length / BLOCK_SIZE; j++)
    {
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            block[i] = data[BLOCK_SIZE * j + i];
        }
        for (int i = 9; i >= 1; i--)
        {
            X_Transformation(block, BLOCK_SIZE, _subKeys[i]);
            L_Transformation_inverse(block);
            S_Transformation_inverse(block, BLOCK_SIZE);
        }
        X_Transformation(block, BLOCK_SIZE, _subKeys[0]);
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            data[BLOCK_SIZE * j + i] = block[i];
        }
    }
}

/**
 * Функция формирования ключа
 *
 * @param key ключ
 */
void SetKey(uint8_t* key)
{
    uint8_t x[SUB_LENGTH];
    uint8_t y[SUB_LENGTH];
    uint8_t c[SUB_LENGTH];
    for (int i = 0; i < SUB_LENGTH; i++)
    {
        x[i] = key[i];
        y[i] = key[i + 16];
        _subKeys[0][i] = x[i];
        _subKeys[1][i] = y[i];
    }
    for (int k = 1; k < 5; k++)
    {
        for (int j = 1; j <= 8; j++)
        {
            C_Transformation(c, 8 * (k - 1) + j);
            F_Transformation(c, x, y);
        }
        for (int i = 0; i < SUB_LENGTH; i++)
        {
            _subKeys[2 * k][i] = x[i];
        }        
        for (int i = 0; i < SUB_LENGTH; i++)
        {
            _subKeys[2 * k + 1][i] = y[i];
        }
    }
}

/**
 * Функция определения количества не хватающих байт
 * 
 * Для "Кузнечика" необходим массив длиной, кратный 16
 *
 * @param size размер массива
 * @param pad_mode режим дополнения
 * 
 * @return Длину дополнения
 */
uint8_t get_size_pad(uint64_t size, uint8_t pad_mode)
{
    if (pad_mode == PAD_MODE_1)
        // Если дополнение для процедуры 1 не нужно, возвращаем 0
        if ((BLOCK_SIZE - (size % BLOCK_SIZE)) == BLOCK_SIZE)
            return 0;

    if (pad_mode == PAD_MODE_3)
        // Если дополнение для процедуры 3 не нужно, возвращаем 0
        if ((BLOCK_SIZE - (size % BLOCK_SIZE)) == BLOCK_SIZE)
            return 0;
    // Возвращаем длину дополнения
    return BLOCK_SIZE - (size % BLOCK_SIZE);
}

/**
 * Функция формирования массива для "Кузнечика"
 * 
 * Для "Кузнечика" необходим массив длиной, кратным 16
 *
 * @param out_buf выходной массив
 * @param in_buf входной массив
 * @param pad_size длина дополнения
 * @param size длина входного массива
 * @param pad_mode режим дополнения
 */
void set_padding(uint8_t *out_buf, volatile uint8_t *in_buf, uint8_t pad_size, uint64_t size, uint8_t pad_mode)
{
    for (uint64_t i = 0; i < size; i++)
    {
        out_buf[i] = in_buf[i];
    }
    
    if (pad_size > 0)
    {
        if (pad_mode == PAD_MODE_1) // Для процедуры 1
        {
            uint64_t i;
            for (i = size; i < size + pad_size; i++)
                // Пишем все нули
                out_buf[i] = 0x00;
        }
        if (pad_mode == PAD_MODE_2) // Для процедуры 2
        {
            // Пишем единичку в первый бит
            out_buf[size] = 0x80;
            uint64_t i;
            for (i = size + 1; i < size + pad_size; i++)
                // Далее заполняем все остальное нулями
                out_buf[i] = 0x00;
        }
        if (pad_mode == PAD_MODE_3) // Для процедуры 3
        {
            // Пишем единичку в первый бит
            out_buf[size] = 0x80;
            uint64_t i;
            for (i = size + 1; i < size + pad_size; i++)
                // Далее заполняем все остальное нулями
                out_buf[i] = 0x00;
        }
    }
}