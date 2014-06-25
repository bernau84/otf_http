
#include "cloud_http.h"
#include <cstring>

u8           recv_data[1024];
t_CircleBuff recv_buff = { recv_data, sizeof(recv_data), 0, 0, -1 };

const char *html_ex = {
    "HTTP/1.1 200 OK\r\n"
    "Date: Tue, 01 Dec 2009 23:27:30 GMT\r\n"
    "Vary: Accept-Encoding,User-Agent\r\n"
    "Content-Length: 681\r\n"
    "Content-Type: Multipart/mixed; boundary=\"sample_boundary\";\r\n"
    "\r\n"
    "Multipart not supported :(\r\n"
    "--sample_boundary\r\n"
    "Content-Type: text/css; charset=utf-8\r\n"
    "Content-Location: http://localhost:2080/file.css\r\n"
    "\r\n"
    "body\r\n"
    "{\r\n"
    " background-color: yellow;\r\n"
    "}\r\n"
    "--sample_boundary\r\n"
    "Content-Type: application/x-javascript; charset=utf-8\r\n"
    "Content-Location: http://localhost:2080/file.js\r\n"
    "\r\n"
    "alert(\"Hello from a javascript!!!\");\r\n"
    "\r\n"
    "--sample_boundary\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Content-Base: http://localhost:2080/\r\n"
    "\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "    <link rel=\"stylesheet\" href=\"http://localhost:2080/file.css\">\r\n"
    "</head>\r\n"
    "<body>\r\n"
    " Hello from a html\r\n"
    "    <script type=\"text/javascript\" src=\"http://localhost:2080/file.js\"></script>\r\n"
    "</body>\r\n"
    "</html>\r\n"
    "--sample_boundary--\r\n"
    "\r\n"
};


const char *sip_ex = {

    "HTTP/1.1\r\n"
    "Route: <sip:192.168.1.2;lr>\r\n"
    "Via: SIP/2.0/UDP 192.168.1.11:5060;branch=z9hG4bKAy3kYhUOWM_o2\r\n"
    "From: sip:nari@lcc;tag=N6fkYqF4Dw\r\n"
    "To: sip:PoC_Server@localhost.localdomain\r\n"
    "Supported: timer,sec-agree\r\n"
    "Require: pref\r\n"
    "Contact: <sip:nari@192.168.1.11;comp=sigcomp>\r\n"
    "CSeq: 142 INVITE\r\n"
    "Call-ID: W73kYsaFt0E_mzf3WH2IuLjDK050Ol\r\n"
    "Accept-Contact: +g.poc.talkburst\r\n"
    "User-Agent: PoC-Client 1.0\r\n"
    "Subject: PoC Client\r\n"
    "Allow: INVITE,ACK,CANCEL,BYE,NOTIFY\r\n"
    "Max-Forwards: 70\r\n"
    "Content-Type: multipart/mixed\r\n"
    "Content-Length: 686\r\n"
    "\r\n"
    "--boundary1\r\n"
    "Content-Type: application/sdp\r\n"
    "\r\n"
    "v=0\r\n"
    "o=PoC-Client 2890844526 2890842808 IN IP4 192.168.1.11\r\n"
    "s=PoC-Client-Media-Lib\r\n"
    "c=IN IP4 192.168.1.11\r\n"
    "t=0 0\r\n"
    "a=direction:both\r\n"
    "m=application 3456 udp TBCP\r\n"
    "a=sendrecv\r\n"
    "a=fmtp:TBCP queuing=1; priority=2; timestamp=1\r\n"
    "m=audio 60000 RTP/AVP 0 96 97\r\n"
    "a=rtpmap:0 AMR/8000\r\n"
    "a=rtpmap:96 L8/8000\r\n"
    "a=rtpmap:97 L8/16000\r\n"
    "\r\n"
    "--boundary1\r\n"
    "Content-Type: application/resource-lists+xml\r\n"
    "Content-Disposition: recipient-list\r\n"
    "Content-ID:<PoC-list>\r\n"
    "\r\n"
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
    "<resource-lists xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\r\n"
    "<list>\r\n"
    "<entry uri=\"sip:almu@lcc\" />\r\n"
    "<entry uri=\"sip:amador@lcc\" />\r\n"
    "</list>\r\n"
    "</resource-lists>\r\n"
    "--boundary1--\r\n"
    "\r\n"
};


bool parse_http_scenario1(const char *ex, int exsz){

    char rep_count = 0;
    u8 rep_imed[32];

    t_CircleBuff rep_buff = { (u8 *)ex, (u16)exsz, 0, 0, -1 };

    int N = 1666;

    t_http_multipart_parser parser(&recv_buff);

    for(int n=0; n<N; n += sizeof(rep_imed)){

        CircleBuff_Write(&rep_buff, 0,  sizeof(rep_imed));  //imaginary write
        CircleBuff_Read(&rep_buff, rep_imed, sizeof(rep_imed));

        parser.feed(rep_imed, sizeof(rep_imed));
        if(parser.iscomplete()){

            parser.restart();
            rep_count += 1;
        }

    }

    if(rep_count == (recv_buff.size / rep_buff.size))
        return true;  //we've found as many completed http as possible

    return false;
}

bool parse_http_scenario2(){
    //porusena delka
    return false;
}

bool parse_http_scenario3(){
    //vyseparovani priloh
    //vycteni subkeys
    //vycteni contenttypes
    return false;
}


bool parse_http_scenario4(){
    //parsovani na zaklade content-length
    return false;
}


bool http_parser_test1(){

    return parse_http_scenario1(html_ex, strlen(html_ex));
}
