#include "MainWindow.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QScrollBar>
#include <QMimeData>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeySequence> // Required for QKeySequence
#include <QCloseEvent> // Required for closeEvent
#include <QTextDocument> // Required for isModified
#include "LineNumberArea.h" // Include the new header

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), typewriterModeEnabled(false), currentFilePath("") {
    textEdit = new QTextEdit(this);
    // setCentralWidget(textEdit); // Will be replaced by composite widget
    textEdit->setPlaceholderText("Welcome to Alte! Drag and drop a text file here or start typing.");

    lineNumberArea = new LineNumberArea(textEdit);

    QWidget *editorWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(editorWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineNumberArea);
    layout->addWidget(textEdit);
    // editorWidget->setLayout(layout); // QHBoxLayout constructor already sets the parent
    setCentralWidget(editorWidget);


    // File Actions
    newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    openAction = new QAction("Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAction = new QAction("Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    quitAction = new QAction("Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close); // Connect to MainWindow's close

    // Typewriter Mode Action
    typewriterModeAction = new QAction("Typewriter Mode", this);
    typewriterModeAction->setCheckable(true);
    typewriterModeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(typewriterModeAction, &QAction::triggered, this, &MainWindow::toggleTypewriterMode);

    // Menu Bar
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(typewriterModeAction);

    // Connect cursor position changed signal
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateTypewriterCenter);
    connect(textEdit->document(), &QTextDocument::modificationChanged, this, &MainWindow::onModificationChanged);

    // Enable drops on the main window
    setAcceptDrops(true);
    updateWindowTitle(); // Set initial window title
}

MainWindow::~MainWindow() {
}

void MainWindow::updateWindowTitle() {
    QString title;
    QString baseName;
    if (currentFilePath.isEmpty()) {
        baseName = "Untitled";
    } else {
        baseName = QFileInfo(currentFilePath).fileName();
    }
    if (textEdit->document()->isModified()) {
        title = baseName + "* - Alte";
    } else {
        title = baseName + " - Alte";
    }
    setWindowTitle(title);
}

void MainWindow::onModificationChanged(bool modified) {
    updateWindowTitle();
}

bool MainWindow::maybeSave() {
    if (!textEdit->document()->isModified()) {
        return true; // No unsaved changes
    }
    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, tr("Alte"),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return false;
}

void MainWindow::newFile() {
    if (maybeSave()) {
        textEdit->clear();
        currentFilePath.clear();
        textEdit->document()->setModified(false);
        updateWindowTitle();
    }
}

void MainWindow::openFile() {
    if (maybeSave()) {
        QString filePath = QFileDialog::getOpenFileName(this, "Open File", QString(), "Text Files (*.txt);;All Files (*)");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                textEdit->setPlainText(in.readAll());
                file.close();
                currentFilePath = filePath;
                textEdit->document()->setModified(false);
                updateWindowTitle();
                // TODO: Signal syntax highlighter to update based on new file type/name
            } else {
                QMessageBox::warning(this, "Open File", "Could not open file: " + file.errorString());
            }
        }
    }
}

bool MainWindow::saveFile() {
    if (currentFilePath.isEmpty()) {
        return saveFileAs();
    } else {
        QFile file(currentFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << textEdit->toPlainText();
            file.close();
            textEdit->document()->setModified(false);
            // updateWindowTitle(); // Already handled by setModified -> onModificationChanged
            return true;
        } else {
            QMessageBox::warning(this, "Save File", "Could not save file: " + file.errorString());
            return false;
        }
    }
}

bool MainWindow::saveFileAs() {
    QString filePath = QFileDialog::getSaveFileName(this, "Save File As", currentFilePath.isEmpty() ? "." : currentFilePath, "Text Files (*.txt);;All Files (*)");
    if (!filePath.isEmpty()) {
        currentFilePath = filePath;
        // TODO: Update syntax highlighting based on new file name/extension
        // setModified(false) will be called by saveFile(), which also updates title via signal
        return saveFile();
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        event->accept(); // Changes saved or discarded, accept close
    } else {
        event->ignore(); // User cancelled, ignore close
    }
}

void MainWindow::toggleTypewriterMode() {
    typewriterModeEnabled = !typewriterModeEnabled;
    typewriterModeAction->setChecked(typewriterModeEnabled);
    if (typewriterModeEnabled) {
        updateTypewriterCenter();
    } else {
        // Reset any specific scrolling behavior if necessary.
        // QTextEdit might handle this naturally by returning to normal scroll behavior.
        // For now, we don't need to do anything specific here.
    }
}

void MainWindow::updateTypewriterCenter() {
    if (typewriterModeEnabled) {
        QTextCursor cursor = textEdit->textCursor();
        QRect cursorRect = textEdit->cursorRect(cursor);
        int viewportHeight = textEdit->viewport()->height();
        int desiredY = cursorRect.top() - viewportHeight / 2 + cursorRect.height() / 2; // Center the line cursor is on

        textEdit->verticalScrollBar()->setValue(desiredY);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        if (!maybeSave()) {
             event->ignore(); // User cancelled saving current file
             return;
        }
        const QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            const QUrl url = urls.first(); // Process only the first file
            if (url.isLocalFile()) {
                const QString filePath = url.toLocalFile();
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QString fileContent = in.readAll();
                    textEdit->setPlainText(fileContent);
                    currentFilePath = filePath;
                    textEdit->document()->setModified(false);
                    updateWindowTitle();
                    // TODO: Signal syntax highlighter to update based on new file type/name
                    file.close();
                    event->acceptProposedAction();
                } else {
                     QMessageBox::warning(this, "Open File", "Could not open dropped file: " + file.errorString());
                }
            }
        }
    }
}
