#ifndef SHORTCUTSETTINGSDIALOG_H
#define SHORTCUTSETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QKeySequenceEdit>
#include "ShortcutManager.h" // For CommandId and ShortcutInfo

QT_BEGIN_NAMESPACE
class QTableWidget;
class QPushButton;
class QTableWidgetItem;
QT_END_NAMESPACE

class ShortcutSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShortcutSettingsDialog(ShortcutManager* shortcutManager, QWidget *parent = nullptr);
    ~ShortcutSettingsDialog();

private slots:
    void populateTable();
    void onApply();
    void onResetSelectedShortcut();
    void onClearSelectedShortcut();
    void onShortcutInputCellChanged(int row, int column);
    void onCellDoubleClicked(int row, int column);

private:
    void setupUi();
    void connectSignals();
    QString keySequenceToString(const QKeySequence& sequence);
    QKeySequence stringToKeySequence(const QString& string);

    ShortcutManager* m_shortcutManager;
    QTableWidget* m_shortcutsTable;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QPushButton* m_clearButton;

    QMap<QString, ShortcutInfo> m_initialShortcuts; // To track changes and revert on cancel. Key: commandName
    QMap<QString, QString> m_modifiedShortcuts; // To store pending changes. Key: commandName, Value: new shortcut string
};

#endif // SHORTCUTSETTINGSDIALOG_H
