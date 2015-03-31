
#ifndef _CLOUD_HTTPH_
#define _CLOUD_HTTPH_

#include "cloud_mime.h"

/*! \brief - constant string defines searching patterns in http data stream
 and keys for looking in associative content storage of http/mime
*/
extern const char *sHttpVer;
extern const char *sHttpAuth;
extern const char *sHttpType;
extern const char *sHttpLength;
extern const char *sHttpHost;

const t_array_content http_keys_v1_0[] = {

    {sHttpVer, 0},
    {sHttpAuth, 0},
    {sHttpType, 0},
    {sHttpLength, 0},
    {sHttpHost, 0},
    {NULL, 0}  //End Mark
};

/* \class mime_http
 * \brief - control deserialization of all http and mime headers
 * first attachment corresponds to http (potentionaly multipart) header
 * workd on-fly directly on recieved data in circular buffer
 * tries to find all content keys in 'keys' structure
 */

#define MIME_MULTIPART_MAXN   16
class t_http_multipart_parser {

protected:
    t_mime_parser   m_mime[MIME_MULTIPART_MAXN]; //attachments in multipart
    t_mime_parser   m_http; //http header; other than defalt contructor for mime attachment
    t_mime_parser  *m_att;  //working/undone part
    t_CircleBuff    m_buf;  //data raw buffer

    char  boundary[64];
    int   length;

    unsigned short last_err;

public:

    /*! \todo - musime zmenit tak aby sme nemuseli kopirovat data,
     * buffer musi byt mimo tridu a budem volat jen update/refresh
     * ne primo data zapisovat !*/
    void feed(uint8_t *dt, unsigned int size);  //update & parse (inf dt = 0 buffer if filled in advance)

    virtual void result(unsigned short code){  //parsing/processing error
        /*! \todo - shoul return http recive code as well */
        last_err = code;
    }

    t_mime_parser get_head(){  //returns copy of parser parser

        return m_http;
    }

    t_mime_parser get_part(unsigned int n){  //returns copy of parser parser
       
        return m_mime[n % MIME_MULTIPART_MAXN];
    }

    t_assoc_chlist_content<MIME_HEADERS_MAXN> get_content(unsigned int n){  //get cppy of mime content

        return m_mime[n % MIME_MULTIPART_MAXN].content;
    }
    
    int parts(){ //number of avaiable attachments

       return (m_mime - m_att);
    }

    bool iscomplete(){   //http whole?

        return ((last_err) || (m_http.iscomplete()));
    }

    void restart(){   //reset parser; starts looking http header

        boundary[0] = 0;
        length = 0;
        last_err = 0;

        m_http.reset();
        m_http.init(&m_buf);

        m_att = m_mime;

        for(int n=0; n<MIME_MULTIPART_MAXN; n++){

            m_mime[n].reset();
            m_att->init(&m_buf);
        }
    }

    t_http_multipart_parser(t_CircleBuff *_buf):
       m_http(http_keys_v1_0){
        
        m_buf = *_buf;
        restart();
    }
   
    virtual ~t_http_multipart_parser(){;}
};



#endif //_CLOUD_HTTPH_




//GARBAGE
//          char *type;  /*! < inline | attachment | form-data | */
//          char *filename; /*! < filename=filename=abcdef.hij */
//          char *size; /*! < size=258584 */
//          char *name; /*! < name=rpc-list | rpc-argument | authetication */
//          struct date {
//
//            char *creat;  /*! < creation-date= */
//            char *modif;  /*! <  modifiction-date= */
//            char *read; /*! <  read-date= */
//          }
//            "name=",
//            "size=",
//            "filename=",
//            {
//                "creation-date=",
//                "modifiction-date=",
//                "read-date="
//            }
//
//
//            NAME, SIZE, FILENAME,
//            DATE_CREATE, DATE_MOD, DATE_READ,



