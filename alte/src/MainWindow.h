#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QAction;
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void toggleTypewriterMode();
    void updateTypewriterCenter();

private:
    QTextEdit *textEdit;
    bool typewriterModeEnabled;
    QAction *typewriterModeAction;
};

#endif // MAINWINDOW_H
