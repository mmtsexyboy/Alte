#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QTextEdit>
#include <QPaintEvent>

class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(QTextEdit *editor);
    ~LineNumberArea() override;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void updateLineNumberAreaOnScroll();

private:
    int lineNumberAreaWidth();
    QTextEdit *codeEditor;
};

#endif // LINENUMBERAREA_H
