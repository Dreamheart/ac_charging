#ifndef __EEPROM_H__
#define __EEPROM_H__

void EepromWrite ( uint16_t addr, uint8_t *buff, uint16_t len ); //  写缓冲区数据到EEPROM
void EepromWrite ( uint16_t addr, uint8_t *buff, uint16_t len ); // 读EEPROM数据到缓冲区
uint8_t LoadParameter ( void ) ; //读保存的参数
void SaveParameter ( void ); //保存参数
uint8_t LoadAddress ( void )  ; //读保存的地址信息
void SaveAddress ( void )  ;//保存地址信息

uint8_t LoadEthernet ( void );
uint8_t LoadPwrDownPara ( void );
void SavePwrDownPara ( void );
void SaveEthernet ( void );
void Byte4ToPara ( uint32_t *dat, uint8_t *ptr );
void ParaToByte4 ( uint32_t dat, uint8_t *ptr );
uint8_t GetChkSum ( uint8_t *buff, uint16_t len );
#endif




