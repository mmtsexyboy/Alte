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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), typewriterModeEnabled(false) {
    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);
    textEdit->setPlaceholderText("Welcome to Alte! Drag and drop a text file here or start typing.");
    // textEdit->setAcceptDrops(true); // Enable drops on the textEdit - Handled by MainWindow

    // Typewriter Mode Action
    typewriterModeAction = new QAction("Typewriter Mode", this);
    typewriterModeAction->setCheckable(true);
    typewriterModeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(typewriterModeAction, &QAction::triggered, this, &MainWindow::toggleTypewriterMode);

    // Menu Bar
    QMenu *viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(typewriterModeAction);

    // Connect cursor position changed signal
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateTypewriterCenter);

    // Enable drops on the main window
    setAcceptDrops(true);
}

MainWindow::~MainWindow() {
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
                    file.close();
                    event->acceptProposedAction();
                }
            }
        }
    }
}
