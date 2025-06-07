#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QAction;
class QDragEnterEvent;
class QDropEvent;
class LineNumberArea;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void toggleTypewriterMode();
    void updateTypewriterCenter();
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void onModificationChanged(bool modified);

private:
    void updateWindowTitle();
    bool maybeSave();

    QTextEdit *textEdit;
    bool typewriterModeEnabled;
    QAction *typewriterModeAction;

    QString currentFilePath;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *quitAction;

    LineNumberArea *lineNumberArea;
};

#endif // MAINWINDOW_H
