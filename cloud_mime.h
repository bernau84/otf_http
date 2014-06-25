#ifndef CLOUD_MIME_H
#define CLOUD_MIME_H

#include <cstdlib>
#include <cstring>

#include "cloud_helper.h"
#include "cloud_assoc_cont.h"


/*! \brief - constant string defines searching pattern in mime data stream
 and keys for looking in associative content storage of mime
*/
extern const char *sContentType;
extern const char *sContentId;
extern const char *sContentLocation;
extern const char *sContentDisposition;
extern const char *sContentTransferEnc;
extern const char *sBody;
extern const char *sBoundary;

/*! \todo */
#define MIME_TRACE(NO, FORMAT, ARGS...)\
    fprintf (stderr, "%d>  "FORMAT"\n", NO, ARGS);


/*!\brief - structure of mime header */
const t_array_content mime_keys_v1_0[] = {

    { sContentType, 0},
    { sContentId, 0},
    { sContentLocation, 0},
    { sContentDisposition, 0},
    { sContentTransferEnc, 0},
//    { sBody, 0}, - at runtime
//    { sBoundary, 0}, - at runtime
    { NULL, 0}    //End Mark
};

//prototype
class t_http_multipart_parser;

/* \class t_mime_parser
 * \brief - deserialize mime runtime directly on recieved data in circular buffer
 * tries to find all content keys in 'keys' structure
 */


#define MIME_HEADERS_MAXN    8

class t_mime_parser {

  private:
      friend class t_http_multipart_parser;

      enum t_mime_sta {

        STOPED = 0,
        SEARCHBGN,
        HEADPROC,
        BODYPROC,
        COMPLETED,
        OWERFLOW,
        PARSEERR
      } sta;

      t_assoc_chlist_content<MIME_HEADERS_MAXN> content; /*! < mime header in assotiative list  */

      t_CircleBuff   parser_buf;   /*! < data to work on */
      t_CircleBuff   content_buf;   /*! < for marking range of this mime & detect overflow */
public:
      const uint8_t *get_raw(const char *key, uint8_t *it, uint32_t *sz);
      int            get_formated(const char *key, const char *f, ...);
      int            get_keyparam(const char *key, const char *par, const char *f, ...);
      void           refresh(uint32_t nnew);

      uint32_t data_avaiable(){  //check if mime already has data and how many

          uint8_t *p;
          if((p = (uint8_t *)content.get(sBody)->p) == NULL)
            return 0;

          t_CircleBuff lbuf = parser_buf; //on loc. copy
          if(sta != COMPLETED) lbuf.Read_mark = p - lbuf.buf;  //set data pointer
          return CircleBuff_RdFree(&lbuf);
      }

      bool iscomplete(unsigned int size = 0){  //check if mime is complete

          if(sta == COMPLETED) return true;
          return (size) ? (data_avaiable() >= size) : false;
      }

      t_mime_sta status(){

          return sta;
      }

      void reset(){ //recyclate struct for new mime rx

          content.reset();
          sta = (parser_buf.size) ? SEARCHBGN : STOPED;
      }

      void init(t_CircleBuff *_buf){ //asign raw data to mime deserializator

          content_buf = parser_buf = *_buf;  //depp copy to save begin of attachment
          reset();
      }

      void enclose(){  //close mime right now

          sta = COMPLETED;
      }

      void enclose(uint32_t mark){  //define end od mime from outside

          parser_buf.Write_mark = mark % parser_buf.size;
          enclose();
      }

      t_mime_parser(const t_array_content *_keys = mime_keys_v1_0):
          content(_keys){

        reset();
        sta = STOPED;
      }

      ~t_mime_parser(){;}
};

#endif // CLOUD_MIME_H