#pragma once

#include <QWidget>
#include <QTextEdit>

class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(QTextEdit *editor);
    ~LineNumberArea();

    QSize sizeHint() const override;
    int lineNumberAreaWidth();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount = 0);
    void updateLineNumberArea(const QRect &rect = QRect(), int dy = 0);
    void updateLineNumberAreaOnScroll();

private:
    QTextEdit *codeEditor;
};
