
#include "cloud_http.h"
#include <cstring>

#include <QTcpSocket>
#include <QUrl>

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

typedef struct {

    const char *user;
    const char *pass;
    const char *uri;
    const char *method;
    const char *realm;
    const char *nonce;
    const char *qop;  //empty by default, or "auth"
    const char *alg; //"MD5" only
} t_http_auth_props;

static char * cloud_gen_basic_authstr(char *authstr, unsigned authlen,
                                   const t_http_auth_props *prop){

    //use coding\libb64-1.2.1\src\ library

    /*
    snprintf(authstr, authlen,
             "Authorization: Digest\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             ,
             );
    */

}


static char * cloud_gen_digest_authstr(char *authstr, unsigned authlen,
                                   const t_http_auth_props *prop){

    //unsupported version
    if(strstr(prop->qop, "auth-int") != NULL)
        return NULL;

    if(strstr(prop->alg, "MD5-sess") != NULL)
        return NULL;

    char * h1;
    char * h2;
    char * response;

    // spocitat h1, h2, response -- verze qop=auth
    //h1 = md5(username:realm:password);
    char h1buffer[strlen(user) + strlen(realm) + strlen(pass) + 2];
    snprintf(h1buffer, sizeof(h1buffer),
             "%s:%s:%s", user, realm, pass);
    h1 = md5(h1buffer).c_str();

    //h2 = md5(method ("get/post"):digest_uri)
    char h2buffer = new char[strlen(uri) + strlen(method) + 1];
    snprintf(h2buffer, sizeof(h2buffer), "%s:%s", method, uri);
    h2 = md5(h2buffer).c_str();

    //response = md5(h1:nonce:h2)
    if(!qop || !*qop){

        char responsebuffer[2*32 + strlen(nonce) + 2];
        snprintf(responsebuffer, sizeof(responsebuffer),
                 "%s:%s:%s", h1, prop->nonce, h2);
        response = md5(responsebuffer).c_str();
    } else if(strstr(qop, "auth") == 0){

        /*! randomize cnonce + increment cn */
        char responsebuffer[2*32 + strlen(prop->nonce) + strlen(cnonce) + strlen(cn) + 2];
        snprintf(responsebuffer, sizeof(responsebuffer),
                 "%s:%s:%s:%s:%s:%s", h1, prop->nonce, cnonce, nc, prop->qop, h2);
        responce = md5(responsebuffer).c_str();
    }

    snprintf(authstr, authlen,
             "Authorization: Digest\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             "%s,\r\n"
             ,
             );

    return authstr;

}

int cloud_web_http_auth(const char *url, const char *user, const char *pass){

    QUrl dest = QUrl(QString(url));

    QTcpSocket sc();
    sc.connectToHost(dest.host(), dest.port());
    if(!sc.waitForConnected()){
        qDebug() << sc.errorString();
    }

    rxparser.restart();

    QString http_req2, authstr;
    QString http_req1 = QString("GET %1 HTTP/1.1\r\n"
             "Connection: keep-alive\r\n"
             "Host: %2\r\n").arg(dest.path()).arg(dest.host());

    sc.write(http_req1.toLatin1());
    sc.write("\r\n");

    while(1){

        switch(__cloud_web_http_result(sc)){


            case 401: {  //auth required

                char methodstr[32];
                t_mime_parser head = rxparser.get_head();
                head.get_formated(sHttpAuth, "%s", methodstr);
                if(!strcmp(method, "Basic")){
                    /* Basic - code username and password to B64 */
                    /*! todo */
                    Q_UNUSED(user)
                    Q_UNUSED(pass)

                    authstr = "dXNlcjE6dXNlcjE=";

                } else if(!strcmp(method, "Digest")){

                    char realm[64], nonce[64], algorithm[16], domain[32], qop[8];
                    t_mime_parser head = rxparser.get_head();

                    head.get_keyparam(sHttpAuth, "realm=\"", "%[^\"]", realm);
                    head.get_keyparam(sHttpAuth, "nonce=\"", "%[^\"]", nonce);
                    head.get_keyparam(sHttpAuth, "algorithm=", "%[^,]", algorithm);
                    head.get_keyparam(sHttpAuth, "domain=\"", "%[^\"]", domain);
                    head.get_keyparam(sHttpAuth, "qop=\"", "%[^\"]", qop);

                    char nc[16], response[64], cnonce[32];
                    /*! \todo - count digest parameters from given */

                    rxparser.restart();
                    authstr = QString("Authorization: Digest username=\"%1\", realm=\"%2\", nonce=\"%3\", uri=\"/auth-digest/\", algorithm=MD5, response=\"%4\", qop=auth, nc=00000001, cnonce=\"%5\"\r\n"
                                   ).arg("user1").arg(realm).arg(nonce).arg(response).arg(cnonce);

                } else {

                    return 0;
                }

                rxparser.restart();

                http_req2 = http_reg1 + QString("Authorization: %1 %2\r\n").arg(method).arg(authstr);
                sc.write(http_req2.toLatin1());
                sc.write("\r\n");
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


