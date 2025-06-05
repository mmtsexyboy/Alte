#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit> // Changed from QTextEdit
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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Alte Editor"); // Will be updated by newFile()
        setGeometry(100, 100, 800, 600);

        textEdit = new QPlainTextEdit(this); // Changed from QTextEdit
        setCentralWidget(textEdit);
        textEdit->setLayoutDirection(QApplication::layoutDirection()); // Explicitly set layout direction
        highlighter = nullptr; // Initialize to nullptr

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
            // Add sample Persian text for RTL testing
            textEdit->setPlainText("سلام دنیا! این یک متن نمونه فارسی در Alte Editor است.\n"
                                   "ویرایشگر متن Alte.\n"
                                   "خط جدید ۱۲۳.\n"
                                   "English line then some Persian: تست فارسی.\n"
                                   "فارسی سپس انگلیسی: Persian Test.");
            setWindowTitle("Alte Editor - Untitled");
            textEdit->document()->setModified(false);
            updateSyntaxHighlighter(currentFilePath); // Update for new (empty path) file
        }
    }

    void openFile() {
        if (maybeSave()) {
            QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Text Files (*.txt);;C++ Files (*.cpp *.h *.cxx *.hpp);;Python Files (*.py);;JavaScript Files (*.js);;All Files (*)")); // Added more file types
            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    textEdit->setPlainText(in.readAll());
                    file.close();
                    currentFilePath = filePath;
                    setWindowTitle("Alte Editor - " + QFileInfo(filePath).fileName());
                    textEdit->document()->setModified(false);
                    updateSyntaxHighlighter(currentFilePath); // Update for opened file
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
    void updateSyntaxHighlighter(const QString &filePath) {
        QString langName;
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            QString extension = fileInfo.suffix().toLower();
            if (extension == "py" || extension == "pyw") {
                langName = "python";
            } else if (extension == "cpp" || extension == "cxx" || extension == "cc" ||
                       extension == "h" || extension == "hpp" || extension == "hh") {
                langName = "cpp";
            } else if (extension == "js" || extension == "mjs") {
                langName = "javascript";
            }
            // Add more language mappings here if needed
        }
        // If langName is empty, SyntaxHighlighter will be created with no specific language,
        // effectively disabling highlighting or using a default if implemented.

        if (this->highlighter) {
            delete this->highlighter;
            this->highlighter = nullptr;
        }

        if (textEdit && textEdit->document()) { // Ensure document and textEdit exist
            this->highlighter = new SyntaxHighlighter(textEdit->document(), langName);
        }
    }

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
        connect(undoAction, &QAction::triggered, textEdit, &QPlainTextEdit::undo); // Changed to QPlainTextEdit

        redoAction = new QAction(tr("&Redo"), this);
        redoAction->setShortcuts(QKeySequence::Redo);
        connect(redoAction, &QAction::triggered, textEdit, &QPlainTextEdit::redo); // Changed to QPlainTextEdit

        cutAction = new QAction(tr("Cu&t"), this);
        cutAction->setShortcuts(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, textEdit, &QPlainTextEdit::cut); // Changed to QPlainTextEdit

        copyAction = new QAction(tr("&Copy"), this);
        copyAction->setShortcuts(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, textEdit, &QPlainTextEdit::copy); // Changed to QPlainTextEdit

        pasteAction = new QAction(tr("&Paste"), this);
        pasteAction->setShortcuts(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, textEdit, &QPlainTextEdit::paste); // Changed to QPlainTextEdit

        selectAllAction = new QAction(tr("Select &All"), this);
        selectAllAction->setShortcuts(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, textEdit, &QPlainTextEdit::selectAll); // Changed to QPlainTextEdit
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

    QPlainTextEdit *textEdit; // Changed from QTextEdit
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

    QString qss = QLatin1String(
        "/* General Application Styles */\n"
        "QWidget {\n"
        "    /* color: #D4D4D4; */ /* Default text color for widgets - can be overridden */\n"
        "    /* background-color: #1E1E1E; */ /* General background for the application window */\n"
        "}\n"
        "\n"
        "QMainWindow {\n"
        "    background-color: #2D2D2D; /* Background for the main window frame */\n"
        "}\n"
        "\n"
        "QPlainTextEdit {\n"
        "    background-color: #1E1E1E; /* Dark background for editor */\n"
        "    color: #D4D4D4;            /* Default text color */\n"
        "    border: 1px solid #3C3C3C; /* Subtle border */\n"
        "    selection-background-color: #005A9E; /* Selection background (VS Code like blue) */\n"
        "    selection-color: #FFFFFF;          /* Text color for selection */\n"
        "    font-family: \"Monospace\"; /* Suggest a monospace font, system can choose best */\n"
        "    /* font-size: 10pt; */ /* Optional: set a default font size */\n"
        "}\n"
        "\n"
        "/* Menu Bar */\n"
        "QMenuBar {\n"
        "    background-color: #2D2D2D; /* Dark background for menubar */\n"
        "    color: #CCCCCC;            /* Text color for menubar items */\n"
        "    border-bottom: 1px solid #3C3C3C;\n"
        "}\n"
        "QMenuBar::item {\n"
        "    background-color: transparent;\n"
        "    padding: 4px 8px;\n"
        "}\n"
        "QMenuBar::item:selected { /* When selected using keyboard navigation */\n"
        "    background-color: #005A9E;\n"
        "    color: #FFFFFF;\n"
        "}\n"
        "QMenuBar::item:pressed { /* When an item is pressed */\n"
        "    background-color: #004C87;\n"
        "    color: #FFFFFF;\n"
        "}\n"
        "\n"
        "/* Menu */\n"
        "QMenu {\n"
        "    background-color: #2D2D2D; /* Background for dropdown menus */\n"
        "    color: #CCCCCC;            /* Text color for menu items */\n"
        "    border: 1px solid #3C3C3C; /* Border around the dropdown */\n"
        "    padding: 2px;\n"
        "}\n"
        "QMenu::item {\n"
        "    padding: 4px 20px 4px 20px; /* Padding for menu items */\n"
        "}\n"
        "QMenu::item:selected {\n"
        "    background-color: #005A9E;\n"
        "    color: #FFFFFF;\n"
        "}\n"
        "QMenu::separator {\n"
        "    height: 1px;\n"
        "    background-color: #3C3C3C;\n"
        "    margin-left: 10px;\n"
        "    margin-right: 5px;\n"
        "}\n"
        "\n"
        "/* ScrollBars - making them less obtrusive */\n"
        "QScrollBar:vertical {\n"
        "    border: 1px solid #3C3C3C;\n"
        "    background: #2D2D2D;\n"
        "    width: 10px;\n"
        "    margin: 0px 0px 0px 0px;\n"
        "}\n"
        "QScrollBar::handle:vertical {\n"
        "    background: #555555;\n"
        "    min-height: 20px;\n"
        "    border-radius: 5px;\n"
        "}\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {\n"
        "    border: none;\n"
        "    background: none;\n"
        "    height: 0px;\n"
        "}\n"
        "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {\n"
        "    background: none;\n"
        "}\n"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\n"
        "    background: none;\n"
        "}\n"
        "\n"
        "QScrollBar:horizontal {\n"
        "    border: 1px solid #3C3C3C;\n"
        "    background: #2D2D2D;\n"
        "    height: 10px;\n"
        "    margin: 0px 0px 0px 0px;\n"
        "}\n"
        "QScrollBar::handle:horizontal {\n"
        "    background: #555555;\n"
        "    min-width: 20px;\n"
        "    border-radius: 5px;\n"
        "}\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {\n"
        "    border: none;\n"
        "    background: none;\n"
        "    width: 0px;\n"
        "}\n"
        "QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {\n"
        "    background: none;\n"
        "}\n"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {\n"
        "    background: none;\n"
        "}\n"
        "\n"
        "/* QMessageBox for consistent theming if possible (might be limited by native dialogs) */\n"
        "QMessageBox {\n"
        "    background-color: #2D2D2D;\n"
        "    /* color: #D4D4D4; */ /* This might not apply well to all QMessageBox parts */\n"
        "}\n"
        "QMessageBox QLabel { /* For the message text */\n"
        "    color: #D4D4D4;\n"
        "}\n"
        "QMessageBox QPushButton {\n"
        "    background-color: #3C3C3C;\n"
        "    color: #D4D4D4;\n"
        "    border: 1px solid #555555;\n"
        "    padding: 5px 10px;\n"
        "    min-width: 70px;\n"
        "}\n"
        "QMessageBox QPushButton:hover {\n"
        "    background-color: #555555;\n"
        "}\n"
        "QMessageBox QPushButton:pressed {\n"
        "    background-color: #005A9E;\n"
        "}\n"
        "/* QFileDialog is often a native dialog and hard to style comprehensively with QSS. */\n"
        "/* This QSS is a starting point and might need adjustments. */\n"
    ");\n"
    "app.setStyleSheet(qss);\n"
    "\n"
    "MainWindow mainWindow;\n"
    mainWindow.show(); // Show the window first
    mainWindow.newFile(); // Then initialize with newFile to get sample RTL text and set title

    return app.exec();
}
