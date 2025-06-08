#include "ShortcutManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QAction> // Include QAction
#include <QDebug>  // For logging

ShortcutManager::ShortcutManager(QObject *parent)
    : QObject(parent), m_configFilePath("shortcuts.json") {
    initializeDefaultShortcuts();
    // Attempt to load user-defined shortcuts, potentially overriding defaults
    // loadShortcuts(); // We might call this explicitly after MainWindow setup
}

ShortcutManager::~ShortcutManager() {}

void ShortcutManager::initializeDefaultShortcuts() {
    registerCoreShortcut(CommandId::NEW_FILE, "Ctrl+N", "New File");
    registerCoreShortcut(CommandId::OPEN_FILE, "Ctrl+O", "Open File");
    registerCoreShortcut(CommandId::SAVE_FILE, "Ctrl+S", "Save File");
    registerCoreShortcut(CommandId::SAVE_AS_FILE, "Ctrl+Shift+S", "Save File As");
    registerCoreShortcut(CommandId::QUIT_APP, "Ctrl+Q", "Quit Application");
    registerCoreShortcut(CommandId::TOGGLE_TYPEWRITER_MODE, "Ctrl+Shift+T", "Toggle Typewriter Mode");
    registerCoreShortcut(CommandId::OPEN_SHORTCUT_SETTINGS, "Ctrl+Alt+S", "Open Shortcut Settings");
    registerCoreShortcut(CommandId::ZEN_MODE, "F11", "Toggle Zen Mode");
    registerCoreShortcut(CommandId::OPEN_SCRATCHPAD, "Ctrl+Shift+P", "Open Scratchpad");

    qDebug() << "Default shortcuts initialized. Total:" << m_shortcuts.count();
}

bool ShortcutManager::registerCoreShortcut(CommandId id, const QString& defaultSequence, const QString& description, QAction* action) {
    return registerNamedShortcut(commandIdToString(id), defaultSequence, description, action);
}

bool ShortcutManager::registerNamedShortcut(const QString& commandName, const QString& defaultSequence, const QString& description, QAction* action) {
    if (commandName.isEmpty()) {
        qWarning() << "ShortcutManager: Cannot register shortcut with empty command name.";
        return false;
    }
    if (m_shortcuts.contains(commandName)) {
        qWarning() << "ShortcutManager: Command" << commandName << "already registered. Updating description and default sequence.";
        m_shortcuts[commandName].description = description;
        m_shortcuts[commandName].defaultSequence = defaultSequence;
        // If currentSequence was loaded from user config, don't overwrite with default here.
        // It will be handled by loadShortcuts. If not loaded, it should take default.
        if (m_shortcuts[commandName].currentSequence.isEmpty() || m_shortcuts[commandName].currentSequence == m_shortcuts[commandName].defaultSequence) {
             m_shortcuts[commandName].currentSequence = defaultSequence;
        }
    } else {
        m_shortcuts.insert(commandName, ShortcutInfo(commandName, defaultSequence, defaultSequence, description, nullptr));
        qDebug() << "ShortcutManager: Registered" << commandName << "(" << description << ") with default" << defaultSequence;
    }

    if (action) {
        connectToAction(commandName, action);
    }
    return true;
}

void ShortcutManager::connectToAction(const QString& commandName, QAction* action) {
    if (!action) return;

    if (m_shortcuts.contains(commandName)) {
        m_shortcuts[commandName].action = action;
        applyShortcutToAction(commandName, m_shortcuts[commandName].currentSequence);
        // Connect action's triggered signal if we need to intercept or log,
        // but typically QAction handles its own triggering.
    } else {
        qWarning() << "ShortcutManager: Cannot connect QAction. Command" << commandName << "not registered.";
    }
}

void ShortcutManager::applyShortcutToAction(const QString& commandName, const QString& sequenceStr) {
    if (m_shortcuts.contains(commandName) && m_shortcuts[commandName].action) {
        QKeySequence keySeq = QKeySequence::fromString(sequenceStr, QKeySequence::PortableText);
        if (!keySeq.isEmpty() || sequenceStr.isEmpty()) { // Allow empty string to clear shortcut
            m_shortcuts[commandName].action->setShortcut(keySeq);
            qDebug() << "Applied shortcut" << keySeq.toString() << "to action for command" << commandName;
        } else {
            qWarning() << "ShortcutManager: Could not parse key sequence" << sequenceStr << "for command" << commandName;
        }
    }
}

bool ShortcutManager::updateShortcut(const QString& commandName, const QString& newSequenceStr) {
    if (m_shortcuts.contains(commandName)) {
        QKeySequence newKeySeq = QKeySequence::fromString(newSequenceStr, QKeySequence::PortableText);
        // Allow empty sequence to remove a shortcut
        if (newKeySeq.isEmpty() && !newSequenceStr.isEmpty()) {
            qWarning() << "ShortcutManager: Invalid key sequence string provided:" << newSequenceStr;
            return false;
        }

        m_shortcuts[commandName].currentSequence = newSequenceStr; // Store as string
        if (m_shortcuts[commandName].action) {
            m_shortcuts[commandName].action->setShortcut(newKeySeq);
        }
        emit shortcutChanged(commandName, newKeySeq);
        qDebug() << "ShortcutManager: Updated shortcut for" << commandName << "to" << newSequenceStr;
        // Consider saving all shortcuts here or provide a separate save mechanism
        saveShortcuts(m_configFilePath);
        return true;
    }
    qWarning() << "ShortcutManager: Cannot update. Command" << commandName << "not found.";
    return false;
}

QKeySequence ShortcutManager::getShortcut(const QString& commandName) const {
    if (m_shortcuts.contains(commandName)) {
        return QKeySequence::fromString(m_shortcuts[commandName].currentSequence, QKeySequence::PortableText);
    }
    return QKeySequence(); // Return empty sequence if not found
}

QString ShortcutManager::getShortcutString(const QString& commandName) const {
    if (m_shortcuts.contains(commandName)) {
        return m_shortcuts[commandName].currentSequence;
    }
    return QString();
}

QString ShortcutManager::getDefaultShortcutString(const QString& commandName) const {
    if (m_shortcuts.contains(commandName)) {
        return m_shortcuts[commandName].defaultSequence;
    }
    return QString();
}

QString ShortcutManager::getShortcutDescription(const QString& commandName) const {
     if (m_shortcuts.contains(commandName)) {
        return m_shortcuts[commandName].description;
    }
    return "Unknown Command";
}

const QMap<QString, ShortcutInfo>& ShortcutManager::getAllShortcuts() const {
    return m_shortcuts;
}

void ShortcutManager::triggerCommand(const QString& commandName) {
    if (m_shortcuts.contains(commandName) && m_shortcuts[commandName].action) {
        m_shortcuts[commandName].action->trigger();
    } else {
        qWarning() << "ShortcutManager: Cannot trigger. Command" << commandName << "has no action or is not registered.";
        // Here you could also emit a signal that MainWindow or another component connects to
        // if the command doesn't have a direct QAction.
    }
}

void ShortcutManager::loadShortcuts(const QString& filePath) {
    m_configFilePath = filePath.isEmpty() ? m_configFilePath : filePath; // Update path if new one provided

    // Use QStandardPaths for user-specific config location
    QString configDirLocation = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDirLocation.isEmpty()) {
        qWarning() << "ShortcutManager: Could not determine config directory.";
        // Fallback to local directory for portable mode or if standard paths fail
        configDirLocation = QDir::currentPath();
    }
    QDir configDir(configDirLocation);
    if (!configDir.exists()) {
        configDir.mkpath("."); // Create the directory if it doesn't exist
    }
    QString fullPath = configDir.filePath(m_configFilePath);
    qDebug() << "ShortcutManager: Attempting to load shortcuts from" << fullPath;


    QFile file(fullPath);
    if (!file.exists()) {
        qInfo() << "ShortcutManager: Shortcut configuration file" << fullPath << "does not exist. Using defaults.";
        // Apply default sequences to actions if they exist
        for (const QString& cmdName : m_shortcuts.keys()) {
            m_shortcuts[cmdName].currentSequence = m_shortcuts[cmdName].defaultSequence;
            applyShortcutToAction(cmdName, m_shortcuts[cmdName].defaultSequence);
        } //This ensures actions get their default shortcuts if no config file.
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ShortcutManager: Could not open shortcut file for reading:" << file.errorString();
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "ShortcutManager: Failed to parse shortcut JSON:" << parseError.errorString();
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "ShortcutManager: Shortcut JSON is not an object.";
        return;
    }

    QJsonObject shortcutsObj = doc.object();
    for (const QString& key : shortcutsObj.keys()) {
        // Key is the commandName string
        if (m_shortcuts.contains(key)) { // Check if this command is known (i.e., registered)
            QString loadedSequence = shortcutsObj[key].toString();
            if (!loadedSequence.isEmpty()) {
                m_shortcuts[key].currentSequence = loadedSequence;
                qDebug() << "ShortcutManager: Loaded shortcut for command" << key
                         << ":" << loadedSequence;
                applyShortcutToAction(key, loadedSequence);
            } else {
                 // If stored sequence is empty, it means "no shortcut"
                 m_shortcuts[key].currentSequence = ""; // Explicitly set to empty
                 applyShortcutToAction(key, ""); // Apply empty shortcut
                 qDebug() << "ShortcutManager: Empty shortcut for command" << key << "in config. Set to no shortcut.";
            }
        } else {
            qWarning() << "ShortcutManager: Command" << key << "from shortcut file is not registered. Ignoring.";
        }
    }
     qDebug() << "ShortcutManager: Shortcuts loaded successfully from" << fullPath;
}

void ShortcutManager::saveShortcuts(const QString& filePath) {
    m_configFilePath = filePath.isEmpty() ? m_configFilePath : filePath;

    QString configDirLocation = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
     if (configDirLocation.isEmpty()) {
        qWarning() << "ShortcutManager: Could not determine config directory for saving.";
        configDirLocation = QDir::currentPath(); // Fallback
    }
    QDir configDir(configDirLocation);
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            qWarning() << "ShortcutManager: Could not create config directory" << configDirLocation;
            return;
        }
    }
    QString fullPath = configDir.filePath(m_configFilePath);
    qDebug() << "ShortcutManager: Saving shortcuts to" << fullPath;

    QJsonObject shortcutsObj;
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        // Store only if currentSequence differs from default, or store all
        // Storing all current sequences using their string commandName as key.
        shortcutsObj[it.key()] = it.value().currentSequence;
    }

    QJsonDocument doc(shortcutsObj);
    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "ShortcutManager: Shortcuts saved successfully to" << fullPath;
    } else {
        qWarning() << "ShortcutManager: Could not open shortcut file for writing:" << file.errorString();
    }
}
