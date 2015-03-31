
#include "cloud_http.h"

const char *sHttpVer    = "HTTP/";        //version + code follows
const char *sHttpType   = "Content-Type: ";
const char *sHttpLength = "Content-Length: ";
const char *sHttpHost   = "Host: ";
const char *sHttpAuth   = "WWW-Authenticate: ";

/*! \brief - feed mime parset in order
*/
void t_http_multipart_parser::feed(uint8_t *dt, unsigned int size){

    if(last_err != 0)
        return; //http parsing finished (somehow)

    CircleBuff_Write(&m_buf, dt, size);
    CircleBuff_Read(&m_buf, 0, size);  //dummy read to prevent overflow

    m_http.refresh(size);

    if(m_http.status() == t_mime_parser::OWERFLOW){

        m_http.sta = t_mime_parser::PARSEERR; //overflow is transient state only
        result(413); //entity too long
        return;
    }

    //looking for size definition from http head
    if(m_http.data_avaiable()){  //http head is whole

        if((!length && !boundary[0])){ //cache some values

            if(m_http.get_keyparam(sHttpType, "boundary=\"", "%61[^\"]", &boundary[2]) > 0){
                //we have multipart with boundary definition
                boundary[0] = boundary[1] = '-';

                for(int n=0; n<MIME_MULTIPART_MAXN; n++)
                    m_mime[n].content.add(&boundary[0], empty_content);  //on a first free position
            }

            if(m_http.get_formated(sHttpLength, "%d", &length) > 0){
                //we have length to test end of http
            }
        }

        if((!length && !boundary[0])){ //cache some values

            m_http.sta = t_mime_parser::PARSEERR;
            result(411); //length requested
            return;
        }
    }

    m_att->refresh(size); //actual mime part update (including boundary scan)

    if(t_mime_parser::COMPLETED == m_att->status()){ //move to new attachment

       uint8_t bend[2]; uint32_t nbend = sizeof(bend);
       const uint8_t *pbend = m_att->get_raw(&boundary[0], bend, &nbend);
       if(0 == memcmp(pbend, "--", 2)){

           m_http.enclose(m_att->content_buf.Write_mark); //close this http at with last mime position
           result(200); //virtual method
           return;
       }

       if((m_att - m_mime) < (MIME_MULTIPART_MAXN-1)){

           t_mime_parser *prev = m_att;
           (++m_att)->content.succ = &prev->content;  //increment and chain with previous
           m_att->init(&prev->parser_buf);  //begins at the end of previous
       }

    }

    if(length){ //can guard the overal/http length only

        if(m_http.iscomplete(length)){

            uint32_t BodyEnd_mark = (uint8_t *)m_http.content.get(sBody)->p - m_buf.buf;  //sBody must exists now
            BodyEnd_mark = (BodyEnd_mark + length) % m_buf.size; //begin of body + length

            m_http.enclose(BodyEnd_mark); //close this http at begin of body + length
            result(200); //virtual method
            return;
        }

        m_buf.Read_mark = m_buf.Write_mark; //to avoid owerflow
    }
}
