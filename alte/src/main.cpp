#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowTitle("Alte Text Editor");
    window.resize(800, 600);
    window.show();
    return app.exec();
}
