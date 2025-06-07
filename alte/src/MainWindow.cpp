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
#include <QKeySequence>
#include <QCloseEvent>
#include <QTextDocument>
#include <QStatusBar> // Required for statusBar()
#include <QMouseEvent> // Required for QMouseEvent
#include "LineNumberArea.h"
// AlteThemeManager and AlteSyntaxHighlighter are included via MainWindow.h

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), typewriterModeEnabled(false), currentFilePath(""),
      m_themeManager(nullptr), m_syntaxHighlighter(nullptr), m_currentLanguageName("Plain Text"),
      m_languageStatusLabel(nullptr) { // Initialize new member

    // Initialize ThemeManager
    m_themeManager = new AlteThemeManager(this); // `this` for QObject parent
    // ThemeManager constructor now calls loadLanguageDefinitions

    textEdit = new QTextEdit(this);
    textEdit->setPlaceholderText("Welcome to Alte! Drag and drop a text file here or start typing.");

    // Initialize SyntaxHighlighter
    // The initial language can be empty or a default like "Plain Text"
    m_syntaxHighlighter = new AlteSyntaxHighlighter(textEdit->document(), m_themeManager, m_currentLanguageName);

    lineNumberArea = new LineNumberArea(textEdit);

    QWidget *editorWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(editorWidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineNumberArea);
    layout->addWidget(textEdit);
    setCentralWidget(editorWidget);


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
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);

    typewriterModeAction = new QAction("Typewriter Mode", this);
    typewriterModeAction->setCheckable(true);
    typewriterModeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(typewriterModeAction, &QAction::triggered, this, &MainWindow::toggleTypewriterMode);

    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu *viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(typewriterModeAction);

    connect(textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateTypewriterCenter);
    connect(textEdit->document(), &QTextDocument::modificationChanged, this, &MainWindow::onModificationChanged);

    // Language Status Label in StatusBar
    m_languageStatusLabel = new QLabel(this);
    m_languageStatusLabel->setToolTip("Click to change language");
    statusBar()->addPermanentWidget(m_languageStatusLabel);
    m_languageStatusLabel->installEventFilter(this); // For click handling
    updateLanguageStatusLabel(); // Set initial text

    setAcceptDrops(true);
    updateWindowTitle();
}

MainWindow::~MainWindow() {
    // No need to delete m_languageStatusLabel explicitly if it's parented to MainWindow
    // and added to status bar, Qt handles it.
}

void MainWindow::updateLanguageStatusLabel() {
    if (m_languageStatusLabel) {
        m_languageStatusLabel->setText(m_currentLanguageName.isEmpty() ? "Plain Text" : m_currentLanguageName);
    }
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
        return true;
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
        m_currentLanguageName = "Plain Text"; // Default for new files
        if (m_syntaxHighlighter) {
            m_syntaxHighlighter->setCurrentLanguage(m_currentLanguageName, m_themeManager);
        }
        updateWindowTitle();
        updateLanguageStatusLabel(); // Update status bar
    }
}

void MainWindow::loadFileContentAndDetectLanguage(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString firstLine;
        QTextStream in(&file);
        // Efficiently read just the first line for detection if possible
        if (!file.atEnd()) {
            firstLine = in.readLine();
        }
        // Reset stream to read the whole content for the editor
        in.seek(0);
        QString fileContent = in.readAll();

        textEdit->setPlainText(fileContent);
        file.close();

        currentFilePath = filePath;
        textEdit->document()->setModified(false);

        if (m_themeManager) {
            m_currentLanguageName = m_themeManager->detectLanguage(filePath, firstLine);
            qDebug() << "Detected language:" << m_currentLanguageName << "for file" << filePath;
        } else {
            m_currentLanguageName = "Plain Text"; // Fallback
            qWarning() << "ThemeManager not available for language detection.";
        }

        if (m_syntaxHighlighter) {
            m_syntaxHighlighter->setCurrentLanguage(m_currentLanguageName, m_themeManager);
        } else {
            qWarning() << "SyntaxHighlighter not available.";
        }
        updateWindowTitle();
        updateLanguageStatusLabel(); // Update status bar
    } else {
        QMessageBox::warning(this, "Open File", "Could not open file: " + file.errorString());
        m_currentLanguageName = "Plain Text"; // Fallback
        if (m_syntaxHighlighter) {
            m_syntaxHighlighter->setCurrentLanguage(m_currentLanguageName, m_themeManager);
        }
        updateLanguageStatusLabel(); // Update status bar
    }
}

void MainWindow::openFile() {
    if (maybeSave()) {
        QString filePath = QFileDialog::getOpenFileName(this, "Open File", QString(), "Text Files (*.txt);;All Files (*)");
        if (!filePath.isEmpty()) {
            loadFileContentAndDetectLanguage(filePath);
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
        return saveFile();
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::toggleTypewriterMode() {
    typewriterModeEnabled = !typewriterModeEnabled;
    typewriterModeAction->setChecked(typewriterModeEnabled);
    if (typewriterModeEnabled) {
        updateTypewriterCenter();
    } else {
    }
}

void MainWindow::updateTypewriterCenter() {
    if (typewriterModeEnabled) {
        QTextCursor cursor = textEdit->textCursor();
        QRect cursorRect = textEdit->cursorRect(cursor);
        int viewportHeight = textEdit->viewport()->height();
        int desiredY = cursorRect.top() - viewportHeight / 2 + cursorRect.height() / 2;

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
             event->ignore();
             return;
        }
        const QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            const QUrl url = urls.first();
            if (url.isLocalFile()) {
                const QString filePath = url.toLocalFile(); // Single declaration
                loadFileContentAndDetectLanguage(filePath);
                if (QFile::exists(currentFilePath) && currentFilePath == filePath) {
                     event->acceptProposedAction();
                } else {
                     event->ignore();
                }
            } else {
                 event->ignore();
            }
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

// Event filter for language label click
bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_languageStatusLabel && event->type() == QEvent::MouseButtonRelease) {
        // Ensure it's a left click, though MouseButtonRelease doesn't distinguish easily without more event data
        // QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        // if (mouseEvent->button() == Qt::LeftButton) {
        showLanguageMenu();
        return true; // Event handled
        // }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showLanguageMenu() {
    if (!m_themeManager) return;

    QMenu languageMenu(this);
    QStringList availableLanguages = m_themeManager->getAvailableLanguages();
    std::sort(availableLanguages.begin(), availableLanguages.end());

    QAction* currentLangAction = nullptr;
    for (const QString& langName : availableLanguages) {
        QAction* langAction = languageMenu.addAction(langName);
        langAction->setData(langName);
        langAction->setCheckable(true);
        if (langName == m_currentLanguageName) {
            langAction->setChecked(true);
            currentLangAction = langAction;
        }
    }

    connect(&languageMenu, &QMenu::triggered, this, &MainWindow::setCurrentLanguageFromMenu);

    // Position menu below the label
    if (m_languageStatusLabel) {
        QPoint globalPos = m_languageStatusLabel->mapToGlobal(QPoint(0, m_languageStatusLabel->height()));
        languageMenu.exec(globalPos);
    } else {
        languageMenu.exec(QCursor::pos()); // Fallback to cursor position
    }
}

void MainWindow::setCurrentLanguageFromMenu(QAction* action) {
    if (action) {
        QString selectedLanguage = action->data().toString();
        if (selectedLanguage != m_currentLanguageName) {
            m_currentLanguageName = selectedLanguage;
            if (m_syntaxHighlighter) {
                m_syntaxHighlighter->setCurrentLanguage(m_currentLanguageName, m_themeManager);
            }
            updateLanguageStatusLabel();
            qDebug() << "Language manually changed to:" << m_currentLanguageName;
        }
    }
}
