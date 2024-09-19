
void Encrypt(uint8_t* data, int data_length);
void Decrypt(uint8_t* data, int data_length);
void SetKey(uint8_t* key);
uint8_t get_size_pad(uint64_t size, uint8_t pad_mode);
void set_padding(uint8_t *out_buf, volatile uint8_t *in_buf, uint8_t pad_size, uint64_t size, uint8_t pad_mode);
