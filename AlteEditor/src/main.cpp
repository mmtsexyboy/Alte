#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QString>
#include <QFileInfo>
#include <QCloseEvent>
#include <QLocale>    // Required for QLocale
#include <QDir>       // Required for QDir::homePath()
#include "syntaxhighlighter.h" // Added for syntax highlighting
#include "splashscreen.h"      // For SplashScreen
#include "AlteThemeManager.h"  // For ThemeManager
#include <QTimer>              // For QTimer (might be removable if only for old splash)
#include <QScreen>             // For QScreen to center splash
#include <QCoreApplication>    // For applicationDirPath
#include <QIcon>               // For QIcon (moved to top)

// Forward declare AlteThemeManager if its definition isn't needed in this header part
// class AlteThemeManager; // Not needed here as it's in main()

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Updated constructor to accept AlteThemeManager
    MainWindow(AlteThemeManager* themeManager, QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Alte Editor"); // Will be updated by newFile()
        setGeometry(100, 100, 800, 600);
        setWindowIcon(QIcon(":/icons/alte_icon.png")); // Set window icon from QRC

        textEdit = new QTextEdit(this);
        setCentralWidget(textEdit);

        // Initialize highlighter with theme manager and a default language
        // The actual themeManager instance will come from main()
        if (themeManager) {
            // Apply editor-specific font
            QFont editorFont = themeManager->getEditorFont(textEdit->font());
            textEdit->setFont(editorFont);

            // Default to "python" or "cpp" for testing
            highlighter = new SyntaxHighlighter(textEdit->document(), themeManager, "python");
        } else {
            // Fallback if themeManager is somehow null
            qWarning() << "MainWindow: ThemeManager is null, syntax highlighter might not work correctly.";
            // Initialize highlighter with null themeManager and empty language to avoid crash, though it won't highlight.
            highlighter = new SyntaxHighlighter(textEdit->document(), nullptr, "");
        }

        createActions();
        createMenus();

        currentFilePath = QString();
        // newFile() will be called after show, which sets the initial title and content
        // textEdit->document()->setModified(false); // Set in newFile
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        if (maybeSave()) {
            event->accept();
        } else {
            event->ignore();
        }
    }

public slots: // Make file operations public slots
    void newFile() {
        if (maybeSave()) {
            textEdit->clear();
            currentFilePath.clear();
            // Python sample code for testing syntax highlighting
            textEdit->setPlainText(
                "#!/usr/bin/env python3\n\n"
                "class Greeter:\n"
                "    \"\"\"A simple greeter class\"\"\"\n"
                "    def __init__(self, name):\n"
                "        self.name = name  # Instance variable\n\n"
                "    def greet(self, loud=False):\n"
                "        # This is a single line comment\n"
                "        if loud:\n"
                "            greeting = f'HELLO, {self.name.upper()}!'\n"
                "        else:\n"
                "            greeting = f'Hello, {self.name}'\n"
                "        print(greeting) # Print the greeting\n"
                "        return 0.0 # Return a float\n\n"
                "# Main execution\n"
                "if __name__ == \"__main__\":\n"
                "    player = Greeter(\"Alte User\")\n"
                "    player.greet()\n"
                "    player.greet(loud=True)\n"
                "    # Test numbers: 123, 0x1A, 0.45, 1e-3\n"
                "    number_test = 123 + 0x1A - 0.45 * 1e-3\n"
            );
            setWindowTitle("Alte Editor - Untitled.py"); // Suggest .py for Python
            textEdit->document()->setModified(false);
            // If language switching is implemented later, call highlighter->setCurrentLanguage("python", themeManager);
        }
    }

    void openFile() {
        if (maybeSave()) {
            QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Text Files (*.txt);;All Files (*)"));
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    textEdit->setPlainText(in.readAll());
                    file.close();
                    currentFilePath = filePath;
                    setWindowTitle("Alte Editor - " + QFileInfo(filePath).fileName());
                    textEdit->document()->setModified(false);
                } else {
                    QMessageBox::warning(this, tr("Error"), tr("Could not open file: ") + file.errorString());
                }
            }
        }
    }

    // Internal save logic, used by saveFile and saveFileAs
    bool saveFileInternal(const QString &filePath) {
        QFile file(filePath);
        // Ensure text mode for consistent line endings (LF on Unix, CRLF on Windows)
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << textEdit->toPlainText();
            file.close();
            currentFilePath = filePath;
            setWindowTitle("Alte Editor - " + QFileInfo(filePath).fileName());
            textEdit->document()->setModified(false);
            return true;
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not save file: ") + file.errorString());
            return false;
        }
    }

    bool saveFile() {
        if (currentFilePath.isEmpty()) {
            return saveFileAs();
        } else {
            return saveFileInternal(currentFilePath);
        }
    }

    bool saveFileAs() {
        QString initialPath = currentFilePath.isEmpty() ? QDir::homePath() + "/Untitled.txt" : currentFilePath;
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save File As"), initialPath, tr("Text Files (*.txt);;All Files (*)"));
        if (!filePath.isEmpty()) {
            // Optional: Add default extension if missing
            // if (!QFileInfo(filePath).suffix().compare("txt", Qt::CaseInsensitive)) {
            //     filePath += ".txt";
            // }
            return saveFileInternal(filePath);
        }
        return false;
    }

    // Returns true if it's okay to proceed (file not modified, saved, or user chose Discard).
    // Returns false if user chose Cancel or save failed.
    bool maybeSave() {
        if (!textEdit->document()->isModified()) {
            return true;
        }
        const QMessageBox::StandardButton ret =
            QMessageBox::warning(this, tr("Alte Editor"),
                                 tr("The document has been modified.\n"
                                    "Do you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            return saveFile(); // This will return true if save is successful, false otherwise
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Discard:
            return true; // User chose not to save
        default: // Should not happen
            break;
        }
        return false; // Default to not proceeding if something unexpected happens
    }

private:
    void createActions() {
        newAction = new QAction(tr("&New"), this);
        newAction->setShortcuts(QKeySequence::New);
        connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

        openAction = new QAction(tr("&Open..."), this);
        openAction->setShortcuts(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

        saveAction = new QAction(tr("&Save"), this);
        saveAction->setShortcuts(QKeySequence::Save);
        connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

        saveAsAction = new QAction(tr("Save &As..."), this);
        saveAsAction->setShortcuts(QKeySequence::SaveAs);
        connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

        exitAction = new QAction(tr("E&xit"), this);
        exitAction->setShortcuts(QKeySequence::Quit);
        // Use qApp->closeAllWindows() to ensure all window closeEvents are processed
        connect(exitAction, &QAction::triggered, qApp, &QApplication::closeAllWindows);

        // Edit Actions
        undoAction = new QAction(tr("&Undo"), this);
        undoAction->setShortcuts(QKeySequence::Undo);
        connect(undoAction, &QAction::triggered, textEdit, &QTextEdit::undo);

        redoAction = new QAction(tr("&Redo"), this);
        redoAction->setShortcuts(QKeySequence::Redo);
        connect(redoAction, &QAction::triggered, textEdit, &QTextEdit::redo);

        cutAction = new QAction(tr("Cu&t"), this);
        cutAction->setShortcuts(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, textEdit, &QTextEdit::cut);

        copyAction = new QAction(tr("&Copy"), this);
        copyAction->setShortcuts(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, textEdit, &QTextEdit::copy);

        pasteAction = new QAction(tr("&Paste"), this);
        pasteAction->setShortcuts(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, textEdit, &QTextEdit::paste);

        selectAllAction = new QAction(tr("Select &All"), this);
        selectAllAction->setShortcuts(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, textEdit, &QTextEdit::selectAll);
    }

    void createMenus() {
        QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction(newAction);
        fileMenu->addAction(openAction);
        fileMenu->addAction(saveAction);
        fileMenu->addAction(saveAsAction);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAction);

        QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(undoAction);
        editMenu->addAction(redoAction);
        editMenu->addSeparator();
        editMenu->addAction(cutAction);
        editMenu->addAction(copyAction);
        editMenu->addAction(pasteAction);
        editMenu->addSeparator();
        editMenu->addAction(selectAllAction);

        QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
        QAction *zoomInAction = new QAction(tr("Zoom &In"), this);
        zoomInAction->setShortcut(QKeySequence::ZoomIn);
        // connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn); // Placeholder
        zoomInAction->setEnabled(false);
        viewMenu->addAction(zoomInAction);

        QAction *zoomOutAction = new QAction(tr("Zoom &Out"), this);
        zoomOutAction->setShortcut(QKeySequence::ZoomOut);
        // connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut); // Placeholder
        zoomOutAction->setEnabled(false);
        viewMenu->addAction(zoomOutAction);
    }

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
    // QAction *zoomInAction;
    // QAction *zoomOutAction;

    QString currentFilePath;
    SyntaxHighlighter *highlighter; // Member variable for the highlighter
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application language and layout direction for Persian
    QLocale persianLocale(QLocale::Persian, QLocale::Iran);
    QLocale::setDefault(persianLocale);
    app.setLayoutDirection(Qt::RightToLeft);

    // ThemeManager setup
    AlteThemeManager themeManager;
    // Construct path to theme file relative to application executable
    // Assuming themes are in: <app_dir>/../resources/themes/ (common for deployed apps)
    // or <app_dir>/resources/themes/ (common for build dir execution)
    QString themeFilePath = QCoreApplication::applicationDirPath() + "/resources/themes/default_dark_neon.json";
    QFile themeFile(themeFilePath);
    if (!themeFile.exists()) { // Fallback for development environment structure
        themeFilePath = QCoreApplication::applicationDirPath() + "/../resources/themes/default_dark_neon.json";
        if (!QFile(themeFilePath).exists()) {
             themeFilePath = "./resources/themes/default_dark_neon.json"; // Even further fallback
        }
    }

    if (themeManager.loadTheme(themeFilePath)) {
        themeManager.applyTheme(&app);
    } else {
        qWarning() << "Failed to load default_dark_neon.json. Using default Qt appearance.";
        // Optionally, load a known fallback theme packaged internally or use a very basic default palette.
    }

    SplashScreen splash; // SplashScreen is now a QWidget, not QSplashScreen
    // It's important that MainWindow is created *after* the theme is applied,
    // so it picks up themed styles and palette during its construction.
    MainWindow mainWindow(&themeManager); // Pass themeManager to MainWindow

    // Show and center SplashScreen AFTER MainWindow is created but BEFORE it's shown.
    // This ensures splash is on top.
    splash.show();
    if (QScreen *screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->geometry();
        splash.move((screenGeometry.width() - splash.width()) / 2,
                    (screenGeometry.height() - splash.height()) / 2);
    }

    // Connect the splash screen's animationFinished signal to show main window
    QObject::connect(&splash, &SplashScreen::animationFinished, &app, [&]() {
        mainWindow.show();
        mainWindow.newFile(); // Initialize with newFile as before
        mainWindow.activateWindow(); // Ensure main window gets focus
        splash.close(); // Close the splash screen widget
    });

    // splash.startAnimation(1500); // Old animation call
    splash.startCentralColumnAnimation(500); // New animation call with 500ms duration

    return app.exec();
}
