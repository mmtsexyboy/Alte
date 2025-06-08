#ifndef SCRATCHPADDIALOG_H
#define SCRATCHPADDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QTimer> // For debouncing save

QT_BEGIN_NAMESPACE
class QTextEdit;
class QPushButton;
QT_END_NAMESPACE

class ScratchpadDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScratchpadDialog(QWidget *parent = nullptr);
    ~ScratchpadDialog();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void loadContent();
    void saveContent();
    void onTextChanged();

private:
    void setupUi();
    QString getScratchpadFilePath() const;

    QTextEdit* m_textEdit;
    QTimer m_saveDebounceTimer;
};

#endif // SCRATCHPADDIALOG_H
