#include "ScratchpadDialog.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QCloseEvent>

ScratchpadDialog::ScratchpadDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle("Scratchpad");
    setMinimumSize(300, 400);
    // Make it modeless but stay on top
    setModal(false);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);


    m_saveDebounceTimer.setInterval(1500); // Save 1.5 seconds after last edit
    m_saveDebounceTimer.setSingleShot(true);
    connect(&m_saveDebounceTimer, &QTimer::timeout, this, &ScratchpadDialog::saveContent);
    connect(m_textEdit, &QTextEdit::textChanged, this, &ScratchpadDialog::onTextChanged);

    // Initial load attempt in constructor or first showEvent
    // loadContent(); // showEvent is better to ensure parent is set for positioning etc.
}

ScratchpadDialog::~ScratchpadDialog() {
    // Ensure content is saved on destruction if pending
    if (m_saveDebounceTimer.isActive()) {
        m_saveDebounceTimer.stop();
        saveContent();
    }
}

void ScratchpadDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlaceholderText("Jot down your notes here...");
    mainLayout->addWidget(m_textEdit);
    setLayout(mainLayout);
}

QString ScratchpadDialog::getScratchpadFilePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dataPath.isEmpty()) {
        qWarning() << "Scratchpad: Could not determine AppLocalDataLocation. Using current dir.";
        dataPath = QDir::currentPath(); // Fallback
    }
    QDir dataDir(dataPath);
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
    }
    return dataDir.filePath("alte_scratchpad.txt");
}

void ScratchpadDialog::loadContent() {
    QString filePath = getScratchpadFilePath();
    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_textEdit->setPlainText(in.readAll());
        file.close();
        qDebug() << "Scratchpad content loaded from" << filePath;
    } else {
        qDebug() << "Scratchpad file not found or could not be opened. Starting fresh.";
    }
}

void ScratchpadDialog::saveContent() {
    QString filePath = getScratchpadFilePath();
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        file.close();
        qDebug() << "Scratchpad content saved to" << filePath;
    } else {
        qWarning() << "Could not save scratchpad content to" << filePath << ":" << file.errorString();
    }
}

void ScratchpadDialog::onTextChanged() {
    m_saveDebounceTimer.start(); // Restart timer on each text change
}

void ScratchpadDialog::closeEvent(QCloseEvent *event) {
    if (m_saveDebounceTimer.isActive()) {
        m_saveDebounceTimer.stop(); // Stop timer
        saveContent(); // Save immediately
    }
    QDialog::closeEvent(event);
}

void ScratchpadDialog::showEvent(QShowEvent *event) {
    if (!m_textEdit->document()->isEmpty() && m_textEdit->toPlainText().isEmpty()) {
        // This check is a bit weak, ideally load only once.
        // Or, if you want it to refresh every time it's shown (might not be desired)
    }
    loadContent(); // Load content when dialog is shown
    QDialog::showEvent(event);
}
