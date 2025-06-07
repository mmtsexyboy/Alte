#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QCloseEvent>
#include <QMenuBar>

class QTextEdit;
class QAction;
class QTimer;
class QEvent;

#include "AlteSyntaxHighlighter.h"
class AlteThemeManager;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(AlteThemeManager* p_themeManager, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    void resetTextEditBorderSlot();
    void newFile();
    void openFile();
    bool saveFileInternal(const QString &filePath);
    bool saveFile();
    bool saveFileAs();
    bool maybeSave();

private:
    void createActions();
    void createMenus();
    QString resolveTextEditStyleSheet(bool useGlowColor);
    void applyTextEditFocusGlow();

    QTextEdit *textEdit;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *selectAllAction;

    QString currentFilePath;
    AlteSyntaxHighlighter *highlighter;
    AlteThemeManager* m_themeManager;
    QTimer* m_focusTimer;
    QString m_originalTextEditStyleSheet;
};

#endif // MAINWINDOW_H
