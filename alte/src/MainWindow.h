#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel> // Added
#include <QMenu>  // Added
#include "include/AlteSyntaxHighlighter.h" // Adjusted path
#include "include/AlteThemeManager.h"   // Adjusted path
#include "ShortcutManager.h" // Added for ShortcutManager


QT_BEGIN_NAMESPACE
class QTextEdit;
class QAction;
class QDragEnterEvent;
class QDropEvent;
class LineNumberArea;
// Forward declare QLabel if QStatusBar is not included, but it's better to include for status bar usage.
class ShortcutSettingsDialog; // Forward declaration
class ScratchpadDialog; // Forward declaration
// QStatusBar is part of QMainWindow, so it's available.
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
    bool eventFilter(QObject* watched, QEvent* event) override; // Added

private slots:
    void toggleTypewriterMode();
    void updateTypewriterCenter();
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void onModificationChanged(bool modified);
    void showLanguageMenu(); // Added
    void setCurrentLanguageFromMenu(QAction* action); // Added
    void openScratchpad(); // Added
    void toggleZenMode(); // Added
    void openShortcutSettingsDialog(); // Added

private:
    void updateWindowTitle();
    void updateLanguageStatusLabel(); // Added
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
    QAction* m_openScratchpadAction; // Added
    QAction* m_zenModeAction; // Added
    QAction* m_shortcutSettingsAction; // Added

    LineNumberArea *lineNumberArea;
    QWidget *editorWidget; // Make editorWidget a member to access its layout containing lineNumberArea

    // Language and Syntax Highlighting
    AlteThemeManager* m_themeManager; // Assuming MainWindow owns or has access to this
    AlteSyntaxHighlighter* m_syntaxHighlighter; // Manages syntax highlighting for textEdit
    QString m_currentLanguageName;
    QLabel* m_languageStatusLabel; // Added
    ShortcutManager* m_shortcutManager; // Added

    bool m_isZenModeActive; // Added
    ScratchpadDialog* m_scratchpadDialog; // Added
    // No need to store original visibility, QWidget::isHidden() gives current state before change.
};

#endif // MAINWINDOW_H
