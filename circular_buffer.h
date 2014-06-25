#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define u8  unsigned char
#define s8  signed char
#define u16 unsigned short
#define s16 signed short
#define u32 unsigned int
#define s32 signed int

typedef struct{
  u8 *buf; //pointer na kruhovy buffer
  u16 size;  //velikost bufferu
  u16 Read_mark;    //pozice od ktere muzu cist
  u16 Write_mark;   //pozice od ktere muzu zapisovat
  s16 overflow;      //preteceni bufferu
} t_CircleBuff;

inline u32 CircleBuff_RdFree(t_CircleBuff *buf){

  s32 tmp = (s32)(buf->Write_mark - buf->Read_mark);
  if( ((tmp == 0)&&(buf->overflow >= 0)) || (tmp < 0) ) return(tmp + buf->size);   //volny prostor je prelozeny, nebo doslo k preteceni
  return(tmp);   //prostor nebyl prelozeny (rd muze != wr i pri overflow)
}

inline u32 CircleBuff_WrFree(t_CircleBuff *buf){

  s32 tmp = (s32)(buf->Read_mark - buf->Write_mark);
  if(buf->overflow < 0){ if(tmp > 0) return(tmp); else return(tmp + buf->size); }
  return 0; //mam tu preteceni - nelze zapsat nic; az dokud si app overflow neshodi
}

inline int CircleBuff_Read(t_CircleBuff *buf, u8 *mem, u32 num){

  int tmp = CircleBuff_RdFree(buf); if(tmp > (int)num) tmp = num;
  for(int i = 0; i<tmp; i += 1){

    if(mem){ *mem = buf->buf[buf->Read_mark]; num -= 1; mem += 1; }
    if((buf->Read_mark += 1) >= buf->size) buf->Read_mark = 0;
  }
  if(tmp) buf->overflow = -1;
  return tmp;
}


inline int CircleBuff_Write(t_CircleBuff *buf, u8 *mem, u32 num){

  int tmp = CircleBuff_WrFree(buf); if(tmp > (int)num) tmp = num;
  for(int i = 0; i<tmp; i += 1){

    if(mem){ buf->buf[buf->Write_mark] = *mem; num -= 1; mem += 1; }
    if((buf->Write_mark += 1) >= buf->size) buf->Write_mark = 0;
  }
  if(buf->Read_mark == buf->Write_mark) buf->overflow += (num - tmp);  //pokud se neco neveslo
  return tmp;
}


#endif // CIRCULAR_BUFFER_H
