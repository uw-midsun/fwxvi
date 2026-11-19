#include <QApplication>
#include <QStringList>
#include "gui/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;

    window.show();

    const QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        window.loadLog(arguments.at(1));
    }

    return app.exec();
}
