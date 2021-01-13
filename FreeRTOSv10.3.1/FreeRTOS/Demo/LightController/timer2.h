

#pragma once

#include <stdint.h>

/*
 * \function set_timer2_freq
 * 
 * \args
 *      uint16_t Prescaler  Valor do prescaler
 *      uint32_t fout       Frequ�ncia pretendida
 * 
 * \returns
 *      0       Sucesso
 *      -1      Valor imposs�vel para o prescaler
 * 
 * Configura o Timer2 para gerar contagens � frequ�ncia fout
 */
int8_t set_timer2_freq(uint16_t Prescaler, uint32_t fout);

int8_t config_timer2(uint16_t Prescaler, uint16_t ValPR2);

int8_t set_timer2_int_freq(uint16_t Prescaler, uint32_t fout);

int8_t config_timer2_int(uint16_t Prescaler, uint16_t ValPR2);

void timer2control(uint8_t trun);