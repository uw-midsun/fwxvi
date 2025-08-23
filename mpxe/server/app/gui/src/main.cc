#include <QApplication>
#include "main_window.h"

// TODO: TESTING FIX LATER
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppState state;
    state.payload[QStringLiteral("project_name")]        = QStringLiteral("Test Project");
    state.payload[QStringLiteral("hardware_model")]      = QStringLiteral("HW-1234");
    state.payload[QStringLiteral("version")]             = QStringLiteral("1.0.0");
    state.payload[QStringLiteral("project_status")]      = QStringLiteral("RUNNING");
    state.payload[QStringLiteral("created_at")]          = QStringLiteral("2025-08-23");
 
    QVariantMap afeMap;
    afeMap.insert(QStringLiteral("cell1"), QStringLiteral("on"));
    afeMap.insert(QStringLiteral("cell2"), QStringLiteral("off"));
    state.payload[QStringLiteral("afe")] = afeMap;

    // Construct and show main window
    MainWindow win(state);
    win.show();
   

    return app.exec();
}
