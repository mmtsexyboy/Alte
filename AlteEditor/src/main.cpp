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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Alte Editor"); // Will be updated by newFile()
        setGeometry(100, 100, 800, 600);

        textEdit = new QTextEdit(this);
        setCentralWidget(textEdit);
        highlighter = new SyntaxHighlighter(textEdit->document()); // Initialize highlighter

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

    MainWindow mainWindow;
    mainWindow.show(); // Show the window first
    mainWindow.newFile(); // Then initialize with newFile to get sample RTL text and set title

    return app.exec();
}
