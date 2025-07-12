#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "codebook.h"
#include <cstdint>

uint8_t * write_to_buffer(full_code code, uint8_t * where);
uint8_t * read_from_buffer(byte_code &code, uint8_t * where);

#endif