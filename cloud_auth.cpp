
#include "cloud_http.h"
#include <cstring>

#include <QTcpSocket>

uint8_t           	rxdata[4096];
t_CircleBuff 		rxbuff = { rxdata, sizeof(rxdata), 0, 0, -1 };
t_http_multipart_parser rxparser(&rxbuff);


static int __cloud_web_http_result(QTcpSocket &sc){

    int result = 0;

    while(rxparser.iscomplete() == 0){

        sc.waitForReadyRead(5000);

        int nrx = sc.bytesAvailable();
        if(!nrx) return result;

        char trx[nrx]; sc.read(trx, nrx);
        rxparser.feed((uint8_t *)trx, nrx);
    }

    t_mime_parser head1 = rxparser.get_head();
    head1.get_formated(sHttpVer, "%*f %d", &result);
    rxparser.restart();
    return result;
}


int cloud_web_http_auth_basic(){

    QTcpSocket sc(NULL);
    sc.connectToHost("test.webdav.org", 80);
    if(!sc.waitForConnected()){
        qDebug() << sc.errorString();
    }

    sc.write("GET /auth-basic/ HTTP/1.1\r\n"
               "Host: test.webdav.org\r\n"
               "Connection: keep-alive\r\n"
               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
               "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36\r\n"
               "Accept-Encoding: gzip, deflate, sdch\r\n"
               "Accept-Language: cs-CZ,cs;q=0.8,en;q=0.6\r\n\r\n");

    while(1){

        switch(__cloud_web_http_result(sc)){


            case 401: {  //auth required
                sc.write("GET /auth-basic/ HTTP/1.1\r\n"
                         "Host: test.webdav.org\r\n"
                         "Connection: keep-alive\r\n"
                         "Authorization: Basic dXNlcjE6dXNlcjE=\r\n"
                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
                         "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36\r\n"
                         "Accept-Encoding: gzip, deflate, sdch\r\n"
                         "Accept-Language: cs-CZ,cs;q=0.8,en;q=0.6\r\n\r\n");
            } break;

            case 404:    //we got in
            case 200:
                return 1;
            break;


            case 0:          //timeout
            default:
                return 0;
            break;
        }
    }

}


int cloud_web_http_auth_digest(){

    QTcpSocket sc;
    sc.connectToHost("http://test.webdav.org/auth-digest/", 80);
    return 0;
}
