#include <QCoreApplication>

extern bool http_parser_tests(void);

extern int cloud_web_http_auth_basic(const char *url, const char *user, const char *pass);
extern int cloud_web_http_auth_digest(const char *url, const char *user, const char *pass);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //http_parser_tests();
    //cloud_web_http_auth_basic(0, 0, 0);
    cloud_web_http_auth_digest(0, 0, 0);

    return a.exec();
}
