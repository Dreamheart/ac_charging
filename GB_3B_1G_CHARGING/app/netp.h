#ifndef __NETP_H__
#define __NETP_H__

extern void  mksoe ( int id, int attr );
extern void  mksoeyc ( int id, int value, int attr );

extern void  charge_rcdjudge ( uint8_t flg, uint8_t Mod_NO );
extern void  netp_init ( void );
extern void  netp_main ( void );

#endif