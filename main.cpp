#include <QCoreApplication>

extern bool http_parser_tests(void);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    http_parser_tests();

    return a.exec();
}
