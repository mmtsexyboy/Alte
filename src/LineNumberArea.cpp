#include "LineNumberArea.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

LineNumberArea::LineNumberArea(QTextEdit *editor) : QWidget(editor), codeEditor(editor) {
    // کد اصلاح شده برای سازگاری با Qt5
    connect(codeEditor->document(), &QTextDocument::blockCountChanged, this, &LineNumberArea::updateLineNumberAreaWidth);

    connect(codeEditor->document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, [this](){ update(); });
    connect(codeEditor->verticalScrollBar(), &QScrollBar::valueChanged, this, &LineNumberArea::updateLineNumberAreaOnScroll);
    connect(codeEditor, &QTextEdit::cursorPositionChanged, this, &LineNumberArea::updateLineNumberAreaOnScroll);

    updateLineNumberAreaWidth(0);
}

// Destructor - added as per user's .h file
LineNumberArea::~LineNumberArea() {
    // No specific cleanup needed for raw pointers if ownership is handled by Qt's parent-child system
    // or if codeEditor is owned elsewhere.
}

QSize LineNumberArea::sizeHint() const {
    return QSize(const_cast<LineNumberArea*>(this)->lineNumberAreaWidth(), 0);
}

int LineNumberArea::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, codeEditor->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 5 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 5;
    return space;
}

void LineNumberArea::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setFixedWidth(lineNumberAreaWidth());
    update();
}

void LineNumberArea::updateLineNumberArea(const QRect &/*rect*/, int dy) {
    if (dy != 0) {
        // scroll(0, dy);
    }
    update();
}

void LineNumberArea::updateLineNumberAreaOnScroll() {
    update();
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(110));

    QAbstractTextDocumentLayout *layout = codeEditor->document()->documentLayout();

    // کد اصلاح شده برای سازگاری با Qt5
    QPointF editorViewportOffset(codeEditor->horizontalScrollBar()->value(), codeEditor->verticalScrollBar()->value());

    QTextCursor cursor = codeEditor->cursorForPosition(QPoint(0, static_cast<int>(editorViewportOffset.y())));
    int firstVisibleBlockNumber = cursor.blockNumber();

    painter.setFont(codeEditor->font());
    painter.setPen(Qt::darkGray);

    QRectF currentBlockRect = layout->blockBoundingRect(codeEditor->document()->findBlockByNumber(firstVisibleBlockNumber));
    int currentY = static_cast<int>(currentBlockRect.top() - editorViewportOffset.y() + codeEditor->contentsMargins().top());

    int blockCount = codeEditor->document()->blockCount();
    for (int blockNumber = firstVisibleBlockNumber; blockNumber < blockCount; ++blockNumber) {
        if (currentY > event->rect().bottom()) {
            break;
        }

        QTextBlock block = codeEditor->document()->findBlockByNumber(blockNumber);
        if (!block.isValid() || !block.isVisible()) {
            continue;
        }

        QString number = QString::number(blockNumber + 1);
        painter.drawText(0, currentY, width() - 5, fontMetrics().height(), Qt::AlignRight, number);

        currentBlockRect = layout->blockBoundingRect(block.next());
        if (!block.next().isValid()) break;
        currentY = static_cast<int>(currentBlockRect.top() - editorViewportOffset.y() + codeEditor->contentsMargins().top());
    }
}
