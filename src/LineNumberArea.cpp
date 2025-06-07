#include "LineNumberArea.h" // Will be found due to include_directories(include)
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout> // Required for documentLayout
#include <QDebug> // For debugging output

LineNumberArea::LineNumberArea(QTextEdit *editor) : QWidget(editor), codeEditor(editor) {
    // Set a background color (optional, can be styled from theme later)
    // setAttribute(Qt::WA_StyledBackground, true);
    // setStyleSheet("background-color: #f0f0f0;"); // Example color

    connect(codeEditor, &QTextEdit::blockCountChanged, this, &LineNumberArea::updateLineNumberAreaWidth);

    // Using contentsChange for updates can be complex.
    // For scrolling, connecting directly to the scrollbar is often more reliable.
    // For text changes causing layout shifts (like adding/removing lines not at the end),
    // blockCountChanged and cursorPositionChanged might be more relevant or simpler to handle.
    // The update() call in updateLineNumberAreaOnScroll and updateLineNumberAreaWidth
    // will trigger paintEvent, which recalculates visible lines.
    connect(codeEditor->document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, this, [this](){ update(); });
    connect(codeEditor->verticalScrollBar(), &QScrollBar::valueChanged, this, &LineNumberArea::updateLineNumberAreaOnScroll);
    // Connect to cursorPositionChanged to update if the cursor movement implies a scroll or view change not caught by scrollbar.
    connect(codeEditor, &QTextEdit::cursorPositionChanged, this, &LineNumberArea::updateLineNumberAreaOnScroll);


    updateLineNumberAreaWidth(0); // Initial width calculation
}

QSize LineNumberArea::sizeHint() const {
    // This cast is necessary because lineNumberAreaWidth is not const.
    // It's better to make lineNumberAreaWidth const if it doesn't modify member state,
    // or use a const_cast here if modification is unavoidable but logically const in this context.
    // For now, let's assume it can be const. If not, this needs adjustment.
    return QSize(const_cast<LineNumberArea*>(this)->lineNumberAreaWidth(), 0);
}

// Helper function to calculate width
int LineNumberArea::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, codeEditor->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // Adjust space based on font metrics
    int space = 5 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 5; // Left padding, digit space, right padding
    return space;
}

void LineNumberArea::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setFixedWidth(lineNumberAreaWidth());
    update(); // Repaint with new width
}

// This slot is primarily for handling cases where content changes might affect layout
// without necessarily changing scroll position (e.g., text wrapping changes).
// For simple line additions/removals, blockCountChanged and scrollbar changes cover most needs.
void LineNumberArea::updateLineNumberArea(const QRect &/*rect*/, int dy) {
    if (dy != 0) {
        // This direct scroll might be too simplistic if line heights vary greatly
        // or if other complex layout changes occur.
        // The paintEvent is designed to redraw based on visible blocks,
        // so a simple update() triggered by scrollbar/cursor changes is often enough.
        // scroll(0, dy); // Potentially problematic, prefer relying on paintEvent triggered by update().
    }
    update(); // General repaint for other changes
}

void LineNumberArea::updateLineNumberAreaOnScroll() {
    update(); // Trigger a repaint, paintEvent will draw the correct numbers.
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(110)); // Background color

    QAbstractTextDocumentLayout *layout = codeEditor->document()->documentLayout();
    QPointF editorViewportOffset = codeEditor->contentOffset(); // Physical scroll position in pixels

    // Calculate the first visible block based on the viewport's top edge
    QTextCursor cursor = codeEditor->cursorForPosition(QPoint(0, static_cast<int>(editorViewportOffset.y())));
    int firstVisibleBlockNumber = cursor.blockNumber();

    painter.setFont(codeEditor->font());
    // TODO: Get this color from the theme
    painter.setPen(Qt::darkGray);

    QRectF currentBlockRect = layout->blockBoundingRect(codeEditor->document()->findBlockByNumber(firstVisibleBlockNumber));

    // Calculate the initial Y position for drawing the first line number.
    // This is the top of the first visible block, adjusted by how much it's scrolled off the top of the viewport.
    int currentY = static_cast<int>(currentBlockRect.top() - editorViewportOffset.y() + codeEditor->contentsMargins().top());

    int blockCount = codeEditor->document()->blockCount();
    for (int blockNumber = firstVisibleBlockNumber; blockNumber < blockCount; ++blockNumber) {
        if (currentY > event->rect().bottom()) {
            break; // Stop if we've drawn past the bottom of the paint event's rectangle
        }

        QTextBlock block = codeEditor->document()->findBlockByNumber(blockNumber);
        if (!block.isValid() || !block.isVisible()) { // Should always be valid if blockNumber < blockCount
            continue;
        }

        QString number = QString::number(blockNumber + 1);
        // Draw text aligned to the right, with some padding.
        // fontMetrics().height() gives a decent approximation for the line height.
        painter.drawText(0, currentY, width() - 5, fontMetrics().height(), Qt::AlignRight, number);

        // Move to the next block's Y position
        currentBlockRect = layout->blockBoundingRect(block.next());
        if (!block.next().isValid()) break; // No more blocks
        currentY = static_cast<int>(currentBlockRect.top() - editorViewportOffset.y() + codeEditor->contentsMargins().top());
    }
}
