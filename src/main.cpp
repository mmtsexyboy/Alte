#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QKeySequence>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QPalette>      // Required for QPalette
#include "AlteSyntaxHighlighter.h"
#include "AlteTheme.h"

// Global variables
QString currentFilePath;
// bool documentModified = false; // No longer needed directly, use textEdit->document()->isModified()
QMainWindow *globalWindow = nullptr;
QPlainTextEdit *globalTextEdit = nullptr;
AlteSyntaxHighlighter *globalHighlighter = nullptr;
AlteTheme *currentTheme = nullptr;

void updateWindowTitle() {
    if (!globalWindow || !globalTextEdit) return;
    QString baseTitle = "Alte - ";
    QString fileName = currentFilePath.isEmpty() ? "Untitled" : QFileInfo(currentFilePath).fileName();
    QString modifiedStar = globalTextEdit->document()->isModified() ? "*" : "";
    globalWindow->setWindowTitle(baseTitle + fileName + modifiedStar);
}

// Returns true on success/no-op, false on failure/cancel
bool actualSaveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(globalWindow, "Error", "Could not save file: " + file.errorString());
        return false;
    }
    QTextStream out(&file);
    out << globalTextEdit->toPlainText();
    file.close();
    globalTextEdit->document()->setModified(false); // This will trigger updateWindowTitle via its signal connection
    currentFilePath = filePath; // Ensure currentFilePath is updated after a successful save
    return true;
}

bool saveFileAsLogic() {
    QString filePath = QFileDialog::getSaveFileName(globalWindow, "Save File As", currentFilePath.isEmpty() ? QDir::homePath() : currentFilePath);
    if (filePath.isEmpty()) {
        return false;
    }
    // currentFilePath = filePath; // currentFilePath is updated in actualSaveToFile upon success
    return actualSaveToFile(filePath);
}

bool saveFileLogic() {
    if (currentFilePath.isEmpty()) {
        return saveFileAsLogic();
    } else {
        return actualSaveToFile(currentFilePath);
    }
}

bool maybeSave() {
    if (!globalTextEdit || !globalTextEdit->document()->isModified()) {
        return true; // Nothing to save or no editor
    }
    const QMessageBox::StandardButton ret =
        QMessageBox::warning(globalWindow, "Alte", "The document has been modified.\n"
                               "Do you want to save your changes?",
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFileLogic();
    case QMessageBox::Discard:
        globalTextEdit->document()->setModified(false); // Mark as not modified if discarded
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        return false;
    }
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    currentTheme = new AlteTheme();
    QString themeFilePath = QCoreApplication::applicationDirPath() + "/../resources/themes/default_dark.json";
    QFile themeFileCheck(themeFilePath);
    if (!themeFileCheck.exists()) {
        themeFilePath = "../resources/themes/default_dark.json";
        if (!QFile(themeFilePath).exists()){
            themeFilePath = "./resources/themes/default_dark.json";
        }
    }
    if (!currentTheme->loadThemeFromFile(themeFilePath)) {
        qWarning() << "Failed to load default theme from" << themeFilePath << ". Using fallback colors.";
    }

    QMainWindow window;
    globalWindow = &window;

    QPlainTextEdit *textEdit = new QPlainTextEdit();
    globalTextEdit = textEdit;
    window.setCentralWidget(textEdit);

    QPalette basePal = window.palette(); // Get a base palette
    QPalette editorPal = textEdit->palette();

    editorPal.setColor(QPalette::Base, currentTheme->getColor("background"));
    editorPal.setColor(QPalette::Text, currentTheme->getColor("text"));
    textEdit->setPalette(editorPal);

    // Apply to QMainWindow as well for more consistency
    QPalette windowPal = window.palette();
    windowPal.setColor(QPalette::Window, currentTheme->getColor("background")); // Background of the window
    windowPal.setColor(QPalette::WindowText, currentTheme->getColor("text"));   // General text (e.g. status bar if added)
    // You might want to theme menuBar, statusBar explicitly if they don't inherit well
    window.setPalette(windowPal);


    globalHighlighter = new AlteSyntaxHighlighter(textEdit->document(), currentTheme);

    textEdit->document()->setModified(false);
    // documentModified = false; // Not needed
    updateWindowTitle(); // Set initial title: "Alte - Untitled"

    QObject::connect(textEdit->document(), &QTextDocument::modificationChanged, [&](bool modified) {
        // documentModified = modified; // Not needed
        updateWindowTitle();
    });

    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *openAction = new QAction("&Open...", &window);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    QObject::connect(openAction, &QAction::triggered, [&]() {
        if (!maybeSave()) { // Use maybeSave
            return;
        }

        QString filePath = QFileDialog::getOpenFileName(&window, "Open File", QDir::homePath());
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                textEdit->setPlainText(in.readAll());
                file.close();
                currentFilePath = filePath;
                textEdit->document()->setModified(false);

                if (QFileInfo(currentFilePath).suffix() == "py") {
                    QString pythonSyntaxFile = QCoreApplication::applicationDirPath() + "/../resources/syntax/python.json";
                    QFile syntaxFileObj(pythonSyntaxFile);
                    if (!syntaxFileObj.exists()) {
                        pythonSyntaxFile = "../resources/syntax/python.json";
                        if (!QFile(pythonSyntaxFile).exists()){
                             pythonSyntaxFile = "./resources/syntax/python.json";
                        }
                    }
                     if (!QFile(pythonSyntaxFile).exists()){
                        qWarning() << "python.json not found at checked paths for syntax highlighting.";
                        globalHighlighter->loadRulesFromLanguageFile("");
                    } else {
                        globalHighlighter->loadRulesFromLanguageFile(pythonSyntaxFile);
                    }
                } else {
                    globalHighlighter->loadRulesFromLanguageFile("");
                }
            } else {
                QMessageBox::warning(&window, "Error", "Could not open file: " + file.errorString());
            }
        }
    });

    QAction *saveAction = new QAction("&Save", &window);
    saveAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(saveAction);
    QObject::connect(saveAction, &QAction::triggered, [&]() {
        saveFileLogic(); // Directly call the refactored saveFileLogic
    });

    QAction *saveAsAction = new QAction("Save &As...", &window);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    fileMenu->addAction(saveAsAction);
    QObject::connect(saveAsAction, &QAction::triggered, [&]() {
        if (saveFileAsLogic()) { // Call refactored saveFileAsLogic
            // After saving as, if it's a python file, ensure highlighting is (re)applied
            // This logic is now part of the open/load process, but ensure title is updated
            updateWindowTitle();
            // Syntax highlighting should be re-evaluated if the file extension changed affecting syntax.
            // For simplicity, we assume loadRulesFromLanguageFile is called if necessary during open.
            // If saving an untitled .txt as .py, highlighting won't auto-apply without reopening or new logic.
            // For now, this is acceptable.
             if (QFileInfo(currentFilePath).suffix() == "py") {
                 QString pythonSyntaxFile = QCoreApplication::applicationDirPath() + "/../resources/syntax/python.json";
                 QFile syntaxFileObj(pythonSyntaxFile);
                 if (!syntaxFileObj.exists()) {
                        pythonSyntaxFile = "../resources/syntax/python.json";
                        if (!QFile(pythonSyntaxFile).exists()){
                             pythonSyntaxFile = "./resources/syntax/python.json";
                        }
                 }
                 if (!QFile(pythonSyntaxFile).exists()){
                     qWarning() << "python.json not found after Save As. Highlighting may not be correct.";
                     globalHighlighter->loadRulesFromLanguageFile("");
                 } else {
                    globalHighlighter->loadRulesFromLanguageFile(pythonSyntaxFile);
                 }
            } else {
                 globalHighlighter->loadRulesFromLanguageFile("");
            }
        }
    });

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("E&xit", &window);
    exitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(exitAction, &QAction::triggered, [&]() {
        if (maybeSave()) { // Use maybeSave
            QApplication::quit();
        }
    });
    fileMenu->addAction(exitAction);

    // Note: Proper handling of the window's 'X' button (closeEvent) to also trigger maybeSave()
    // would ideally require subclassing QMainWindow. Without it, clicking 'X' might close
    // the window and quit the app if it's the last window, potentially bypassing maybeSave
    // if not handled by QApplication::aboutToQuit signal (which is more complex to cancel from).
    // For this iteration, we rely on File > Exit or the user explicitly saving.

    updateWindowTitle(); // Ensure title is set before showing
    window.show();
    return app.exec();
}
