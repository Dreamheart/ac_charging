#ifndef __AT45DB081_H__
#define __AT45DB081_H__

void SaveRecord ( uint8_t *buff );
void LoadRecord ( uint32_t num, uint8_t *buff );
void SaveEvent (uint8_t num, uint8_t *buff,uint8_t len );
void LoadEvent ( uint32_t num, uint8_t *buff ,uint8_t len );
void SaveAlarm ( uint8_t *buff );
void LoadAlarm ( uint32_t num, uint8_t *buff );
void FlashInit ( void );
void ErasingDataFlash ( void );
uint16_t GetRecordNum ( void );
uint16_t GetEventNum ( void );
uint16_t GetAlarmNum ( void );
void ClearRecord ( void );
extern uint32_t RecordNum, RecordFlag; //当前记录号，记录满500条标记
extern uint32_t EventNum, EventFlag;
extern uint32_t AlarmNum, AlarmFlag;
#endif