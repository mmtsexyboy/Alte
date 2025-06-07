#include "MainWindow.h" // Use "include/" prefix
#include <QMenuBar>     // For menuBar() -> addMenu()
#include "AlteSyntaxHighlighter.h" // Already in .h but good for cpp if direct methods used
#include "AlteThemeManager.h"    // Already in .h but good for cpp if direct methods used

#include <QApplication> // For qApp
#include <QTextEdit>    // For textEdit member & methods
#include <QMenu>        // For menuBar()->addMenu()
#include <QAction>      // For QAction members
#include <QFileDialog>  // For file dialogs
#include <QFile>        // For QFile
#include <QTextStream>  // For QTextStream
#include <QMessageBox>  // For QMessageBox
#include <QFileInfo>    // For QFileInfo
#include <QDir>         // For QDir
#include <QDebug>       // For qWarning, qDebug
#include <QTimer>       // For m_focusTimer
#include <QFont>        // For QFont in constructor
#include <QMimeData>    // For QDragEnterEvent, QDropEvent
#include <QUrl>         // For QDragEnterEvent, QDropEvent
#include <QDragEnterEvent> // For dragEnterEvent parameter
#include <QDropEvent>   // For dropEvent parameter
#include <QScrollBar>   // For textEdit->verticalScrollBar()

// Constructor Implementation
MainWindow::MainWindow(AlteThemeManager* p_themeManager, QWidget *parent)
    : QMainWindow(parent), m_themeManager(p_themeManager), m_focusTimer(nullptr), typewriterModeEnabled(false) {
    setWindowTitle("Alte Editor"); // Will be updated by newFile()
    setWindowIcon(QIcon(":/icons/alte_icon.png")); // Set window icon from QRC

    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);

    // Initialize highlighter with theme manager and a default language
    if (m_themeManager) {
        // Apply editor-specific font
        QFont editorFont = m_themeManager->getEditorFont(textEdit->font());
        textEdit->setFont(editorFont);

        // Default to "python" or "cpp" for testing
        highlighter = new AlteSyntaxHighlighter(textEdit->document(), m_themeManager, "python");

        m_originalTextEditStyleSheet = resolveTextEditStyleSheet(false); // false for not using glow color
        textEdit->setStyleSheet(m_originalTextEditStyleSheet); // Apply it once to be sure

    } else {
        qWarning() << "MainWindow: ThemeManager is null, syntax highlighter and focus glow might not work correctly.";
        // Fallback if themeManager is somehow null - ensure highlighter is still created
        highlighter = new AlteSyntaxHighlighter(textEdit->document(), nullptr, ""); // Pass nullptr for themeManager
    }

    textEdit->installEventFilter(this);
    m_focusTimer = new QTimer(this);
    m_focusTimer->setSingleShot(true);
    connect(m_focusTimer, &QTimer::timeout, this, &MainWindow::resetTextEditBorderSlot);
    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateTypewriterCenter);

    setAcceptDrops(true); // Enable Drag & Drop
    createActions();
    createMenus();

    currentFilePath = QString();
    // newFile() will be called after show, which sets the initial title and content
    // textEdit->document()->setModified(false); // Set in newFile
}

// Destructor Implementation
MainWindow::~MainWindow() {
    // delete highlighter; // Removed: highlighter is a child of textEdit->document()
    // highlighter is a child of textEdit->document() due to the
    // QSyntaxHighlighter(QTextDocument *parent) constructor.
    // Qt's parent-child mechanism will handle its deletion when the
    // document (and subsequently textEdit via its own parentage to MainWindow) is destroyed.
    // Explicitly deleting it here would likely cause a double free.

    // m_focusTimer is parented to this, will be deleted by Qt.
    // textEdit is parented to this, will be deleted by Qt.
    // All QAction members are parented to this, will be deleted by Qt.
}

// closeEvent Implementation
void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// eventFilter Implementation
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == textEdit) {
        if (event->type() == QEvent::FocusIn) {
            applyTextEditFocusGlow();
            m_focusTimer->start(250); // Glow duration in ms
        } else if (event->type() == QEvent::FocusOut) {
            m_focusTimer->stop();
            resetTextEditBorderSlot(); // Use the slot directly
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// resetTextEditBorderSlot Implementation
void MainWindow::resetTextEditBorderSlot() {
    if (m_themeManager && !m_originalTextEditStyleSheet.isEmpty()) {
        textEdit->setStyleSheet(m_originalTextEditStyleSheet);
    } else if (m_themeManager) { // Fallback if m_originalTextEditStyleSheet was empty
        textEdit->setStyleSheet(resolveTextEditStyleSheet(false));
    }
    // If m_themeManager is null, no easy way to reset to a themed style.
}

// newFile Implementation
void MainWindow::newFile() {
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

// openFile Implementation
void MainWindow::openFile() {
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

// saveFileInternal Implementation
bool MainWindow::saveFileInternal(const QString &filePath) {
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

// saveFile Implementation
bool MainWindow::saveFile() {
    if (currentFilePath.isEmpty()) {
        return saveFileAs();
    } else {
        return saveFileInternal(currentFilePath);
    }
}

// saveFileAs Implementation
bool MainWindow::saveFileAs() {
    QString initialPath = currentFilePath.isEmpty() ? QDir::homePath() + "/Untitled.txt" : currentFilePath;
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File As"), initialPath, tr("Text Files (*.txt);;All Files (*)"));
    if (!filePath.isEmpty()) {
        return saveFileInternal(filePath);
    }
    return false;
}

// maybeSave Implementation
bool MainWindow::maybeSave() {
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
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    case QMessageBox::Discard:
        return true;
    default:
        break;
    }
    return false;
}

// createActions Implementation
void MainWindow::createActions() {
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
    connect(exitAction, &QAction::triggered, qApp, &QApplication::closeAllWindows);

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

    typewriterModeAction = new QAction(tr("Typewriter Mode"), this);
    typewriterModeAction->setCheckable(true);
    typewriterModeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(typewriterModeAction, &QAction::triggered, this, &MainWindow::toggleTypewriterMode);
}

// createMenus Implementation
void MainWindow::createMenus() {
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
    viewMenu->addSeparator(); // Optional: add a separator before the new action
    viewMenu->addAction(typewriterModeAction);
}

// resolveTextEditStyleSheet Implementation
QString MainWindow::resolveTextEditStyleSheet(bool useGlowColor) {
    if (!m_themeManager) return "";

    QString baseStyle = m_themeManager->getStyleSheet("QPlainTextEdit, QTextEdit");
    if (baseStyle.isEmpty()) {
        qWarning() << "resolveTextEditStyleSheet: Could not get base style for QPlainTextEdit, QTextEdit";
        return QString("border: 1px solid %1;").arg(useGlowColor ? m_themeManager->getColor("cyberPulse").name() : m_themeManager->getColor("border").name());
    }

    QColor borderColorToUse = useGlowColor ? m_themeManager->getColor("cyberPulse") : m_themeManager->getColor("border");
    baseStyle.replace("%%border%%", borderColorToUse.name());
    baseStyle.replace("%%alternateBase%%", m_themeManager->getColor("alternateBase").name());
    baseStyle.replace("%%lightMist%%", m_themeManager->getColor("lightMist").name());
    baseStyle.replace("%%cyberPulse%%", m_themeManager->getColor("cyberPulse").name());
    baseStyle.replace("%%highlightedText%%", m_themeManager->getColor("highlightedText").name());

    return baseStyle;
}

// applyTextEditFocusGlow Implementation
void MainWindow::applyTextEditFocusGlow() {
    if (!m_themeManager) return;
    if (m_originalTextEditStyleSheet.isEmpty()) {
         m_originalTextEditStyleSheet = resolveTextEditStyleSheet(false);
    }
    QString glowStyle = resolveTextEditStyleSheet(true);
    textEdit->setStyleSheet(glowStyle);
}

void MainWindow::toggleTypewriterMode() {
    typewriterModeEnabled = !typewriterModeEnabled;
    typewriterModeAction->setChecked(typewriterModeEnabled);
    if (typewriterModeEnabled) {
        updateTypewriterCenter(); // Initial centering when enabled
    }
    // Optional: May need to trigger a repaint or update of the textEdit if styling changes
}

void MainWindow::updateTypewriterCenter() {
    if (typewriterModeEnabled) {
        if (!textEdit) return; // Ensure textEdit is valid
        QTextCursor cursor = textEdit->textCursor();
        QRect cursorRect = textEdit->cursorRect(cursor);

        QWidget *viewport = textEdit->viewport();
        if (!viewport) return;
        int viewportHeight = viewport->height();

        // The y-coordinate of the cursor in the document.
        int cursorY = textEdit->cursorRect().top();
        // The current value of the vertical scrollbar.
        // int scrollY = textEdit->verticalScrollBar()->value(); // Not directly needed for setValue

        // Calculate the desired new value for the scrollbar to center the cursor line.
        // We want the line where the cursor is to appear at the vertical center of the viewport.
        // cursorY is the top of the cursor's bounding rectangle relative to the textEdit's viewport.
        // To center this line, its position (cursorY) when scrolled should be viewportHeight/2 - cursorRect.height()/2.
        // The scrollbar's value represents the top of the visible part of the document.
        // So, if the document content at `newScrollY` is at the top of the viewport,
        // and the cursor is at `cursorY` relative to the viewport,
        // the cursor's position in the document is `newScrollY + cursorY`.
        // We want `newScrollY + cursorY` to be `newScrollY + viewportHeight/2 - cursorRect.height()/2`
        // This means `cursorY` should become `viewportHeight/2 - cursorRect.height()/2`.
        // The current scrollbar value is `textEdit->verticalScrollBar()->value()`.
        // The position of `cursorY` in the document is `textEdit->verticalScrollBar()->value() + cursorY`.
        // We want this document position to be centered.
        // So, `textEdit->verticalScrollBar()->value() + cursorY - (viewportHeight / 2) + (cursorRect.height() / 2)`
        // should be the new scrollbar value.

        int newScrollY = textEdit->verticalScrollBar()->value() + cursorY - (viewportHeight / 2) + (cursorRect.height() / 2);
        textEdit->verticalScrollBar()->setValue(newScrollY);
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
        if (!maybeSave()) { // Use the existing maybeSave method
             event->ignore();
             return;
        }
        const QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            const QUrl url = urls.first(); // Process only the first file for simplicity
            if (url.isLocalFile()) {
                const QString filePath = url.toLocalFile();
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QString fileContent = in.readAll();
                    textEdit->setPlainText(fileContent);
                    currentFilePath = filePath; // Update currentFilePath
                    // Update window title using the logic similar to openFile()
                    setWindowTitle("Alte Editor - " + QFileInfo(filePath).fileName());
                    textEdit->document()->setModified(false);
                    file.close();
                    event->acceptProposedAction();
                } else {
                     QMessageBox::warning(this, tr("Open File Error"), // Use tr() for translatable strings
                                          tr("Could not open dropped file: %1").arg(file.errorString()));
                }
            }
        }
    }
}
