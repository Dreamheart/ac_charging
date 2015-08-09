#ifndef __KEY_H__
#define __KEY_H__

#define KEYMAXNUM       0x0a    
#define KEYSCANCNT      0x05

#define KEYSCANDLY      0x05
#define KEYRPTDLY       0x64

struct KEYBOARD
{ 
    uint8_t keycnt;
    uint8_t status;
    uint8_t dly;
    uint8_t flagfull;
    uint8_t product;
    uint8_t consume;
    uint8_t value[KEYMAXNUM];
    uint8_t code;
    uint32_t scanticks;
};

extern struct KEYBOARD KeyBoardType;

extern void KeyBoard_Scan(void);
#endif