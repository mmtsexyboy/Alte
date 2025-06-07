#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString> // For currentFilePath and m_originalTextEditStyleSheet
#include <QCloseEvent> // For closeEvent method
#include <QMenuBar> // For menuBar()

// Forward declarations
class QTextEdit;
class QAction;
// class QMenu; // Not strictly needed as member, used as local var in private method
class QTimer;
class QEvent; // For eventFilter method

#include "AlteSyntaxHighlighter.h" // Full definition for member variable
// Forward declare custom classes used as pointers
// class AlteSyntaxHighlighter; // Replaced with full include
class AlteThemeManager; // Forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Updated constructor to accept AlteThemeManager
    MainWindow(AlteThemeManager* p_themeManager, QWidget *parent = nullptr);
    ~MainWindow(); // Destructor

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
    void resetTextEditBorderSlot();
    void newFile();
    void openFile();
    bool saveFileInternal(const QString &filePath); // Keep public if main.cpp needs it, otherwise private/protected
    bool saveFile();
    bool saveFileAs();
    bool maybeSave(); // Keep public if main.cpp needs it

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
    // Edit Action members
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *selectAllAction;
    // View Action members (if they need to be accessed later, otherwise can be local in createMenus)
    // QAction *zoomInAction; // Not used as member
    // QAction *zoomOutAction; // Not used as member

    QString currentFilePath;
    AlteSyntaxHighlighter *highlighter; // Member variable for the highlighter
    AlteThemeManager* m_themeManager;
    QTimer* m_focusTimer;
    QString m_originalTextEditStyleSheet;
};

#endif // MAINWINDOW_H
