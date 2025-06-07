#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QTextEdit> // Or forward-declare if only pointers/references are used in header

class LineNumberArea : public QWidget {
    Q_OBJECT

public:
    LineNumberArea(QTextEdit *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void updateLineNumberAreaOnScroll(); // Custom slot for scrollbar valueChanged

private:
    int lineNumberAreaWidth(); // Helper function to calculate width
    QTextEdit *codeEditor;
};

#endif // LINENUMBERAREA_H
