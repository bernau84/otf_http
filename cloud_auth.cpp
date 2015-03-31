
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

    t_mime_parser head = rxparser.get_head();
    head.get_formated(sHttpVer, "%*f %d", &result);
    return result;
}


int cloud_web_http_auth_basic(const char *url, const char *user, const char *pass){

    QTcpSocket sc(NULL);
    sc.connectToHost("test.webdav.org", 80);
    if(!sc.waitForConnected()){
        qDebug() << sc.errorString();
    }

    rxparser.restart();
    sc.write("GET /auth-basic/ HTTP/1.1\r\n"
             "Connection: keep-alive\r\n"
             "Host: test.webdav.org\r\n\r\n");

    while(1){

        switch(__cloud_web_http_result(sc)){


            case 401: {  //auth required

                /*! todo - code username and password to B64 */

                rxparser.restart();
                sc.write("GET /auth-basic/ HTTP/1.1\r\n"
                         "Host: test.webdav.org\r\n"
                         "Connection: keep-alive\r\n"
                         "Authorization: Basic dXNlcjE6dXNlcjE=\r\n\r\n");
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


int cloud_web_http_auth_digest(const char *url, const char *user, const char *pass){

    QTcpSocket sc(NULL);
    sc.connectToHost("test.webdav.org", 80);
    if(!sc.waitForConnected()){
        qDebug() << sc.errorString();
    }

    rxparser.restart();
    sc.write("GET /auth-digest/ HTTP/1.1\r\n"
               "Host: test.webdav.org\r\n"
               "Connection: keep-alive\r\n\r\n");

    while(1){

        switch(__cloud_web_http_result(sc)){


            case 401: {  //auth required

                char realm[64], nonce[64], algorithm[16], domain[32], qop[8];
                t_mime_parser head = rxparser.get_head();

                head.get_formated(sHttpAuth, "realm=\"", "%[^\"]", realm);
                head.get_formated(sHttpAuth, "nonce=\"", "%[^\"]", nonce);
                head.get_formated(sHttpAuth, "algorithm=\"", "%[^\"]", algorithm);
                head.get_formated(sHttpAuth, "domain=\"", "%[^\"]", domain);
                head.get_formated(sHttpAuth, "qop=\"", "%[^\"]", qop);

                char nc[16], response[64], cnonce[32];
                /*! \todo - count digest parameters from given */

                rxparser.restart();
                QString dig_req = QString("GET /auth-digest/ HTTP/1.1\r\n"
                                            "Host: test.webdav.org\r\n"
                                            "Connection: keep-alive\r\n"
                                            "Authorization: Digest username=\"%1\", realm=\"%1\", nonce=\"%1\", uri=\"/auth-digest/\", algorithm=MD5, response=\"%1\", qop=auth, nc=00000001, cnonce=\"%1\"\r\n"
                                          ).arg("user1").arg(realm).arg(nonce).arg(response).arg(cnonce);

                sc.write(dig_req.toLatin1());
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
