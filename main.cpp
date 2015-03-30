#include <QCoreApplication>

extern bool http_parser_tests(void);

extern int cloud_web_http_auth_basic();
extern int cloud_web_http_auth_digest();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //http_parser_tests();
    cloud_web_http_auth_basic();

    return a.exec();
}
