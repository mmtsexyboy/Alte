#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QKeySequence>
#include <QMap>
#include <QString>

class QAction; // Forward declaration

// Enum to define command identifiers
enum class CommandId {
    NEW_FILE,
    OPEN_FILE,
    SAVE_FILE,
    SAVE_AS_FILE,
    QUIT_APP,
    TOGGLE_TYPEWRITER_MODE,
    OPEN_SHORTCUT_SETTINGS,
    ZEN_MODE,
    OPEN_SCRATCHPAD
    // Add more commands as needed
};

// Helper to convert CommandId to its string representation
inline QString commandIdToString(CommandId id) {
    switch (id) {
        case CommandId::NEW_FILE: return "core.newFile";
        case CommandId::OPEN_FILE: return "core.openFile";
        case CommandId::SAVE_FILE: return "core.saveFile";
        case CommandId::SAVE_AS_FILE: return "core.saveAsFile";
        case CommandId::QUIT_APP: return "core.quitApp";
        case CommandId::TOGGLE_TYPEWRITER_MODE: return "core.toggleTypewriterMode";
        case CommandId::OPEN_SHORTCUT_SETTINGS: return "core.openShortcutSettings";
        case CommandId::ZEN_MODE: return "core.zenMode";
        case CommandId::OPEN_SCRATCHPAD: return "core.openScratchpad";
        default: return "unknown.command";
    }
}

// Structure to hold shortcut information
struct ShortcutInfo {
    QString commandName;     // Unique string identifier (e.g., "core.newFile", "plugin.myAction")
    QString defaultSequence; // Default key sequence string
    QString currentSequence; // Current key sequence string (user-defined or default)
    QString description;     // User-friendly description for UI
    QAction* action;         // Associated QAction, if any (can be null)

    ShortcutInfo(QString name = "", QString defSeq = "", QString curSeq = "", QString desc = "", QAction* act = nullptr)
        : commandName(name), defaultSequence(defSeq), currentSequence(curSeq.isEmpty() ? defSeq : curSeq), description(desc), action(act) {}
};

class ShortcutManager : public QObject {
    Q_OBJECT

public:
    explicit ShortcutManager(QObject *parent = nullptr);
    ~ShortcutManager();

    void loadShortcuts(const QString& filePath = "shortcuts.json");
    void saveShortcuts(const QString& filePath = "shortcuts.json");

    // Register using CommandId (for core actions)
    bool registerCoreShortcut(CommandId id, const QString& defaultSequence, const QString& description, QAction* action = nullptr);
    // Register using string command name (for plugins or dynamic actions)
    bool registerNamedShortcut(const QString& commandName, const QString& defaultSequence, const QString& description, QAction* action = nullptr);

    bool updateShortcut(const QString& commandName, const QString& newSequence);
    QKeySequence getShortcut(const QString& commandName) const;
    QString getShortcutString(const QString& commandName) const;
    QString getDefaultShortcutString(const QString& commandName) const;
    QString getShortcutDescription(const QString& commandName) const;
    const QMap<QString, ShortcutInfo>& getAllShortcuts() const; // Key is commandName

    void connectToAction(const QString& commandName, QAction* action);
    void triggerCommand(const QString& commandName);

signals:
    void shortcutChanged(const QString& commandName, const QKeySequence& newSequence);

private:
    void initializeDefaultShortcuts();
    void applyShortcutToAction(const QString& commandName, const QString& sequence);
    QMap<QString, ShortcutInfo> m_shortcuts; // Key is commandName (e.g., "core.newFile")
    QString m_configFilePath;
};

#endif // SHORTCUTMANAGER_H
