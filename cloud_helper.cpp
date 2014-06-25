
#include <cstdlib>
#include <cstring>
#include "stdint.h"

#include "circular_buffer.h"

/*! \brief - helper memcpy on circ buffer
 * do not shift read pointer!
 * \return 0 - match, -2 - low data in buffer, -1 - if no comparable input,
 *     x > 0 - if content differs in byte x-1
 */
int wrapped_memcmp(t_CircleBuff *buf, const uint8_t *key, uint32_t keylen){

  if(!buf || !key)
    return -1;  //sanity check

  if(keylen > CircleBuff_RdFree(buf))
    return -2;  //lenght mismatch

  for(unsigned i=0; i<keylen; i++)
    if(key[i] != buf->buf[(buf->Read_mark + i) % buf->size])
      return i+1;  //mismatch

  return 0; //match
}

/*! \brief - helper strstr like function on circ buffer
 * do not shift read pointer!
 * \return Read_index of findings!
*/
int wrapped_find(t_CircleBuff *buf, const uint8_t *key, uint32_t keylen){

  if(!buf || !key)
    return -2;  //sanity check

  const uint8_t *tkey = key;
  const uint8_t *ch, *first_match = NULL;
  uint32_t read_mark = buf->Read_mark;

  while(read_mark != buf->Write_mark){

      ch = &buf->buf[read_mark];
      if(*tkey == *ch){

        if(tkey == key)
            first_match = ch; //begin of match

        if((unsigned)((tkey += 1) - key) == keylen) //next char
            return first_match - buf->buf;   //finding
 
      } else if(first_match){
        
        tkey = key;  //start from last point
        read_mark = first_match - buf->buf;
        first_match = 0;
      }

      read_mark = (read_mark + 1) % buf->size;
  }

  return -1; //not find
}


/*! \brief - helper strstr like function on linear buffer
 * \return index of findings!
 */
int memfind(const uint8_t *lin, uint32_t linlen, const uint8_t *key, uint32_t keylen){

  if(!lin || !key)
    return -2;  //sanity check

  if(linlen < keylen)
    return -1; //length differences

  for(unsigned i=0; i<(linlen - keylen); i++)
      if(*key == lin[i])
          if(memcmp(&lin[i], key, keylen) == 0)
              return i;

  return -1; //not find
}

