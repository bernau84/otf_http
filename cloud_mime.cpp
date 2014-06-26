#include <cstdio>
#include <cstdarg>

#include "cloud_mime.h"

const char *sContentType    = "Content-Type: ";
const char *sContentId      = "Content-Id: ";
const char *sContentLocation       = "Content-Location: ";
const char *sContentDisposition    = "Content-Disposition: ";
const char *sContentTransferEnc    = "Content-transfer-encoding: ";

const char *sBody     = "__BODY__";   //Body placeholder
const char *sBoundary = "__BOUNDARY__";   //Boundary placeholder

/*! \brief privides deep copy if demanded space is wrapped and it & sz is set
 * otherwise return pointer to inner buffer
 * omit problem with speed for copying in circular buffer
 */
const uint8_t *t_mime_parser::get_raw(const char *key, uint8_t *it, uint32_t *sz){

    uint8_t *p = (uint8_t *)content.get(key)->p;  //return NULL content id do not exists

    if(!it || !sz || !p)
      return p;

    t_CircleBuff lbuf = parser_buf;
    lbuf.Read_mark = p - lbuf.buf;

    if((lbuf.Read_mark + *sz) < lbuf.size)
      return p;   //space is unwrapped, return directy

    //deep copy
    *sz = CircleBuff_Read(&lbuf, (uint8_t *)it, *sz);
    return it;
}

/*! \brief - read given content item
 * suppose we can get into size limit of item
 */
#define MIME_CONTENT_ITSZ   256
int t_mime_parser::get_formated(const char *key, const char *f, ...){

    uint8_t lin[MIME_CONTENT_ITSZ]; //prep[size] variable size doesnt work with vsnprintf ?!
    uint32_t n = sizeof(lin);
    const uint8_t *p = get_raw(key, lin, &n);
    if(NULL == p)
      return -2;  //no such item key yet

    va_list argptr;
    va_start(argptr, f);
    n = vsscanf((char *)p, f, argptr);
    va_end(argptr);
    return n;
}

/*! \brief - read subjkye into given content item
 * suppose we can get into size limit of item
 */
int t_mime_parser::get_keyparam(const char *key, const char *subkey, const char *f, ...){

    uint8_t lin[MIME_CONTENT_ITSZ+1]; //prep[size] variable size doesnt work with vsnprintf ?!
    uint32_t n = sizeof(lin);
    const uint8_t *p = get_raw(key, lin, &n);
    if(NULL == p)
      return -2;  //no such item key yet

    int sub_i = memfind(p, n, (uint8_t *)subkey, strlen(subkey));
    if(sub_i < 0)
      return -1;  //no such subkey

    va_list argptr;
    va_start(argptr, f);
    n = vsscanf((char *)(p+sub_i+strlen(subkey)), f, argptr);  //
    va_end(argptr);
    return n;
}

/*! \brief - update state of keys; it should be called it every new received data
 * calls on-fly parser
 */
void t_mime_parser::refresh(uint32_t nnew){

    switch(sta){

        case SEARCHBGN: case HEADPROC: case BODYPROC:
            //update work & data buffer
            CircleBuff_Write(&parser_buf, 0, nnew);  //shift write pointer
            content_buf.Write_mark = parser_buf.Write_mark;
            break;
        default: //STOPED, OVERFLOW, PARSEERR
            return;
    }

    if(content_buf.overflow >= 0){

        sta = OWERFLOW;
        return;
    }

  MIME_DESER_PARSER:

    t_CircleBuff lbuf = parser_buf;  //work on copy of original
    t_array_content lnew;
    const char *pk = NULL;  //pntr to key name
    int32_t sk = 0, Header_mark = -1;  //index of found key in buffer

    //parser - testing new lines for matching mime content keys
    for(int mi = 0; (NULL != (pk = content.v[mi].key.p)); mi++){

        if(NULL != content.get(pk)->p)  //we already had
            continue;

        if((sk = content.v[mi].key.size) == 0)  //no keylen defined
            continue;

        if(BODYPROC == sta)  //during body look only for boundary, if exists, we cand uses sBoundary key as it is
            if((pk[0] != '-') || (pk[1] != '-'))  //replaced by real roundary strinch which we doesnt know all from here
                continue;

        if(0 != wrapped_memcmp(&lbuf, (uint8_t *)pk, sk))  //match with mk header(== key)
            continue;

        //new match - now we test if the item is completed
        Header_mark = lbuf.Read_mark;  //match mark, remember position
        CircleBuff_Read(&lbuf, NULL, sk);  //go behing key on local copy
        break; //bring new line
    }

    if((BODYPROC == sta) && (pk)) //if body part and no boundary than parses is no need
        return;

#ifdef MIME_TRACE
    char dline[32]; sscanf((char *)&parser_buf.buf[parser_buf.Read_mark], "%31[^\r\n]", dline);
    MIME_TRACE(0, "bgnln - %s(%d)", dline, CircleBuff_RdFree(&parser_buf));
    MIME_TRACE(1, "pttrn - %s", pk);
#endif //MIME_TRACE

    //looking for new line which continues with another enter or letter
    char ch1 = 0, ch2 = 0;
    while((ch2 != '\n') && (ch2 < ' ')){

        //fist - look for new line
        while(ch1 != '\n')
          if(0 == CircleBuff_Read(&lbuf, (uint8_t *)&ch1, 1))
            return; //item si not completed yet

        //second - escape control ascii, stop while white chars and literals
        while((ch2 < ' ') && (ch2 != ' ') && (ch2 != '\t')){  //space or tabs mean continuation of line

            if(0 == CircleBuff_Read(&lbuf, (uint8_t *)&ch2, 1))
                return;

            if(ch2 == '\n'){

                if((sta == HEADPROC) || (sta == SEARCHBGN)){ //set item pointer, length is unknown for now

                    lnew.p = (const char *)&parser_buf.buf[lbuf.Read_mark];
                    lnew.size = (unsigned)-1;
                    content.add(sBody, lnew);
                }
                break;
            }
        }
    }

    //we stay on new header / body now - shift marker
    parser_buf.Read_mark = (lbuf.Read_mark + parser_buf.size - 1) % parser_buf.size;  //move to char before
    if(sta == SEARCHBGN) content_buf.Read_mark = parser_buf.Read_mark;  //for case there is no pk

    if(Header_mark >= 0){  //we've found some mime item - store its props

        switch(sta){

            case SEARCHBGN:
                sta = HEADPROC;
                content_buf.Read_mark = Header_mark;  //fix this position & watch the overflow
            case HEADPROC:
                if(NULL != content.get(sBody, NULL))  //check existence of item Body
                    sta = BODYPROC;
            break;
            case BODYPROC:
                content_buf.Write_mark = parser_buf.Read_mark;
                lbuf.Read_mark = (uint8_t *)content.get(sBody)->p - parser_buf.buf;  //arange markers
                lbuf.Write_mark = Header_mark;
                lnew.p = content.get(sBody)->p; //copy original
                lnew.size = CircleBuff_RdFree(&lbuf); //calculate the body length
                content.set(sBody, lnew); //update item
                sta = COMPLETED; //+ go to set end of boundary item
            break;
            default:
                return;
        }

        Header_mark = (Header_mark + sk) % parser_buf.size;  //move behind header
        lnew.p = (const char *)&parser_buf.buf[Header_mark],  //set item pointer
        lnew.size = (parser_buf.Read_mark + parser_buf.size - Header_mark) % parser_buf.size; //length
        content.set(pk, lnew);
    }

    if(sta != COMPLETED)
        goto MIME_DESER_PARSER; //process new line
}
