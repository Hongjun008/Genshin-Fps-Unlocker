#include "mainwindow.h"
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow window("Genshin Impact FPS Unlocker");
    window.show();

    return QApplication::exec();
}
