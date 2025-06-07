#include "ShortcutSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QPushButton>
#include <QKeySequenceEdit>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

// Custom QKeySequenceEdit that captures key press directly
class CustomKeySequenceEdit : public QKeySequenceEdit {
public:
    CustomKeySequenceEdit(QWidget* parent = nullptr) : QKeySequenceEdit(parent) {
        // setFocusPolicy(Qt::StrongFocus); // Ensure it can get focus
    }

protected:
    // bool event(QEvent *event) override {
    //     if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
    //         QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    //         int k = keyEvent->key();
    //         Qt::KeyboardModifiers m = keyEvent->modifiers();

    //         if (keyEvent->type() == QEvent::KeyPress) {
    //             if (k == Qt::Key_Control || k == Qt::Key_Shift || k == Qt::Key_Alt || k == Qt::Key_Meta) {
    //                 // Just a modifier pressed, store it
    //                 currentModifiers = m;
    //                 setKeySequence(QKeySequence(currentModifiers));
    //                 return true;
    //             }
    //             if (currentModifiers != Qt::NoModifier && k != Qt::Key_unknown) {
    //                  setKeySequence(QKeySequence(currentModifiers | k));
    //             } else if (k != Qt::Key_unknown) {
    //                 setKeySequence(QKeySequence(k));
    //             }
    //         } else { // KeyRelease
    //              if (k == Qt::Key_Control || k == Qt::Key_Shift || k == Qt::Key_Alt || k == Qt::Key_Meta) {
    //                 // Modifier released
    //                 if (keySequence()[0] == currentModifiers) { // if only modifier was set
    //                     // If the sequence was just the modifier, clear it or handle as needed
    //                 }
    //                 currentModifiers = Qt::NoModifier; // Reset
    //              }
    //         }
    //         return true;
    //     }
    //     return QKeySequenceEdit::event(event);
    // }


// void keyPressEvent(QKeyEvent *event) override {
//     QKeySequenceEdit::keyPressEvent(event);
//     if (event->key() == Qt::Key_Backspace && event->modifiers() == Qt::NoModifier) {
//         clear();
//     } else {
//         // This logic is tricky with QKeySequenceEdit as it has its own handling.
//         // It might be better to use a QLineEdit and parse the input manually,
//         // or use a dedicated QKeySequenceEdit and handle its editingFinished signal.
//         // For simplicity, QKeySequenceEdit's default behavior is often sufficient.
//     }
// }

private:
    // Qt::KeyboardModifiers currentModifiers = Qt::NoModifier;
};


ShortcutSettingsDialog::ShortcutSettingsDialog(ShortcutManager* shortcutManager, QWidget *parent)
    : QDialog(parent), m_shortcutManager(shortcutManager) {
    if (!m_shortcutManager) {
        qCritical() << "ShortcutSettingsDialog: ShortcutManager instance is null!";
        // Handle error appropriately, maybe disable the dialog or throw
        return;
    }
    m_initialShortcuts = m_shortcutManager->getAllShortcuts(); // Store initial state
    setupUi();
    populateTable();
    connectSignals();
    setWindowTitle("Configure Shortcuts");
    setMinimumSize(500, 400);
}

ShortcutSettingsDialog::~ShortcutSettingsDialog() {
    // Qt handles child widget deletion
}

void ShortcutSettingsDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_shortcutsTable = new QTableWidget(this);
    m_shortcutsTable->setColumnCount(3); // Command, Current Shortcut, Default Shortcut
    m_shortcutsTable->setHorizontalHeaderLabels({"Command", "Shortcut", "Default"});
    m_shortcutsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_shortcutsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_shortcutsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_shortcutsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Initially no direct edit
    // m_shortcutsTable->setEditTriggers(QAbstractItemView::DoubleClicked); // Enable editing on double click for the shortcut column

    mainLayout->addWidget(m_shortcutsTable);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    m_resetButton = new QPushButton("Reset Selected", this);
    m_clearButton = new QPushButton("Clear Selected", this);
    m_applyButton = new QPushButton("Apply", this);
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Cancel", this);

    buttonsLayout->addWidget(m_resetButton);
    buttonsLayout->addWidget(m_clearButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_applyButton);
    buttonsLayout->addWidget(m_okButton);
    buttonsLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

void ShortcutSettingsDialog::populateTable() {
    m_shortcutsTable->setRowCount(0); // Clear existing rows
    m_modifiedShortcuts.clear(); // Clear pending changes

    const auto& allShortcuts = m_shortcutManager->getAllShortcuts();
    int row = 0;
    for (auto it = allShortcuts.constBegin(); it != allShortcuts.constEnd(); ++it, ++row) {
        m_shortcutsTable->insertRow(row);

        const QString& commandName = it.key();
        const ShortcutInfo& info = it.value();

        QTableWidgetItem* commandItem = new QTableWidgetItem(info.description);
        commandItem->setData(Qt::UserRole, commandName); // Store commandName (QString)
        commandItem->setFlags(commandItem->flags() & ~Qt::ItemIsEditable);
        m_shortcutsTable->setItem(row, 0, commandItem);

        QTableWidgetItem* currentShortcutItem = new QTableWidgetItem(keySequenceToString(QKeySequence::fromString(info.currentSequence, QKeySequence::PortableText)));
        currentShortcutItem->setFlags(currentShortcutItem->flags() | Qt::ItemIsEditable); // Make this cell editable
        m_shortcutsTable->setItem(row, 1, currentShortcutItem);

        QTableWidgetItem* defaultShortcutItem = new QTableWidgetItem(keySequenceToString(QKeySequence::fromString(info.defaultSequence, QKeySequence::PortableText)));
        defaultShortcutItem->setFlags(defaultShortcutItem->flags() & ~Qt::ItemIsEditable);
        m_shortcutsTable->setItem(row, 2, defaultShortcutItem);
    }
    m_shortcutsTable->resizeColumnsToContents();
    m_shortcutsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_shortcutsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}


void ShortcutSettingsDialog::connectSignals() {
    connect(m_okButton, &QPushButton::clicked, this, &ShortcutSettingsDialog::onApply);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &ShortcutSettingsDialog::onApply);
    connect(m_resetButton, &QPushButton::clicked, this, &ShortcutSettingsDialog::onResetSelectedShortcut);
    connect(m_clearButton, &QPushButton::clicked, this, &ShortcutSettingsDialog::onClearSelectedShortcut);
    // connect(m_shortcutsTable, &QTableWidget::cellChanged, this, &ShortcutSettingsDialog::onShortcutInputCellChanged);
    connect(m_shortcutsTable, &QTableWidget::cellDoubleClicked, this, &ShortcutSettingsDialog::onCellDoubleClicked);
}

void ShortcutSettingsDialog::onCellDoubleClicked(int row, int column) {
    if (column == 1) { // Shortcut column
        m_shortcutsTable->editItem(m_shortcutsTable->item(row, column));
        QWidget* editor = m_shortcutsTable->cellWidget(row, column);
        if (!editor) { // If not already a QKeySequenceEdit, create one
            CustomKeySequenceEdit* keyEdit = new CustomKeySequenceEdit(m_shortcutsTable);
            keyEdit->setKeySequence(stringToKeySequence(m_shortcutsTable->item(row, column)->text()));
            m_shortcutsTable->setCellWidget(row, column, keyEdit);
            keyEdit->setFocus(); // Give focus to start editing
            // Connect a signal from the editor to capture the new sequence when editing is finished
            connect(keyEdit, &QKeySequenceEdit::editingFinished, this, [this, row, keyEdit]() {
                QKeySequence newSeq = keyEdit->keySequence();
                QTableWidgetItem* item = m_shortcutsTable->item(row, 1);
                if (item) {
                    item->setText(keySequenceToString(newSeq));
                }
                // Store this change in m_modifiedShortcuts
                QString commandName = m_shortcutsTable->item(row, 0)->data(Qt::UserRole).toString();
                m_modifiedShortcuts[commandName] = newSeq.toString(QKeySequence::PortableText);
                m_shortcutsTable->removeCellWidget(row, 1); // Remove editor, show text
            });
             // Handle Escape key to cancel editing
            QObject::connect(keyEdit, &QKeySequenceEdit::editingFinished, [=]() {
                if (keyEdit->keySequence().isEmpty() && keyEdit->property("userCancelled").toBool()) {
                    // User pressed Esc or cleared it in a way that signifies cancellation
                    m_shortcutsTable->removeCellWidget(row, 1);
                    // Optionally revert to original text if needed, though usually editingFinished handles this
                }
            });
        }
    }
}


void ShortcutSettingsDialog::onShortcutInputCellChanged(int row, int column) {
    if (column != 1) return; // Only care about the "Shortcut" column

    QTableWidgetItem* commandItem = m_shortcutsTable->item(row, 0);
    QTableWidgetItem* shortcutCell = m_shortcutsTable->item(row, column);
    if (!commandItem || !shortcutCell) return;

    QString commandName = commandItem->data(Qt::UserRole).toString();
    QString newSequenceStr = shortcutCell->text();

    // Validate the new sequence string if necessary (QKeySequence::fromString can do this)
    // For now, assume valid input or handle validation in onApply
    m_modifiedShortcuts[commandName] = newSequenceStr; // Store as portable string
    qDebug() << "Shortcut for" << m_shortcutManager->getShortcutDescription(commandName) << "staged to change to" << newSequenceStr;
}


void ShortcutSettingsDialog::onApply() {
    bool changesMade = false;
    for (auto it = m_modifiedShortcuts.constBegin(); it != m_modifiedShortcuts.constEnd(); ++it) {
        const QString& commandName = it.key();
        QString newSequenceStr = it.value();

        // Check if it's actually different from the current one in ShortcutManager
        if (m_shortcutManager->getShortcutString(commandName) != newSequenceStr) {
            if (m_shortcutManager->updateShortcut(commandName, newSequenceStr)) {
                qDebug() << "Applied shortcut for" << m_shortcutManager->getShortcutDescription(commandName) << "to" << newSequenceStr;
                changesMade = true;
            } else {
                qWarning() << "Failed to apply shortcut for" << m_shortcutManager->getShortcutDescription(commandName);
                // Optionally revert this cell in the table or show an error
            }
        }
    }

    if (changesMade) {
        m_shortcutManager->saveShortcuts(); // Persist all changes
        m_initialShortcuts = m_shortcutManager->getAllShortcuts(); // Update initial state to current
        populateTable(); // Refresh table to show current state and clear cell widgets
        QMessageBox::information(this, "Shortcuts Applied", "Shortcut changes have been applied.");
    }
    m_modifiedShortcuts.clear(); // Clear staged changes
}

void ShortcutSettingsDialog::onResetSelectedShortcut() {
    QList<QTableWidgetItem*> selectedItems = m_shortcutsTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "Reset Shortcut", "Please select a command to reset its shortcut.");
        return;
    }

    int row = selectedItems.first()->row();
    QString commandName = m_shortcutsTable->item(row, 0)->data(Qt::UserRole).toString();
    QString defaultSequenceStr = m_shortcutManager->getDefaultShortcutString(commandName);

    QTableWidgetItem* currentShortcutCell = m_shortcutsTable->item(row, 1);
    if (currentShortcutCell) {
        currentShortcutCell->setText(keySequenceToString(QKeySequence::fromString(defaultSequenceStr, QKeySequence::PortableText)));
        m_modifiedShortcuts[commandName] = defaultSequenceStr; // Stage this change
        qDebug() << "Shortcut for" << m_shortcutManager->getShortcutDescription(commandName) << "staged to reset to default:" << defaultSequenceStr;
    }
}

void ShortcutSettingsDialog::onClearSelectedShortcut() {
    QList<QTableWidgetItem*> selectedItems = m_shortcutsTable->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "Clear Shortcut", "Please select a command to clear its shortcut.");
        return;
    }

    int row = selectedItems.first()->row();
    QString commandName = m_shortcutsTable->item(row, 0)->data(Qt::UserRole).toString();

    QTableWidgetItem* currentShortcutCell = m_shortcutsTable->item(row, 1);
    if (currentShortcutCell) {
        currentShortcutCell->setText(""); // Empty string for no shortcut
        m_modifiedShortcuts[commandName] = "";   // Stage this change (empty string)
        qDebug() << "Shortcut for" << m_shortcutManager->getShortcutDescription(commandName) << "staged to be cleared.";
    }
}

QString ShortcutSettingsDialog::keySequenceToString(const QKeySequence& sequence) {
    return sequence.toString(QKeySequence::NativeText);
}

QKeySequence ShortcutSettingsDialog::stringToKeySequence(const QString& string) {
    return QKeySequence::fromString(string, QKeySequence::NativeText);
}

// Override reject to handle pending changes
void ShortcutSettingsDialog::reject() {
    if (!m_modifiedShortcuts.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Discard Changes?",
                                      "You have unsaved shortcut changes. Do you want to discard them?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return; // Don't close
        }
    }
    QDialog::reject(); // Proceed to close
}
