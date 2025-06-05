#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression> // Should be QRegularExpression

class QTextDocument;

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    // Rules for C++
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat functionFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};

#endif // SYNTAXHIGHLIGHTER_H
