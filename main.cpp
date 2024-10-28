#include "mainwindow.h"
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    const char appName[]{"Genshin Impact FPS Unlocker"};
    QApplication::setApplicationName(appName);
    QApplication::setApplicationVersion("0.0.1");
    QApplication::setOrganizationName("Hongjun008");
    QApplication::setOrganizationDomain("hongjun.tech");
    MainWindow window(appName);
    window.show();

    return QApplication::exec();
}
