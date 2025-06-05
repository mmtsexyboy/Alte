#ifndef ALTESYNTAXHIGHLIGHTER_H
#define ALTESYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

// Forward declaration
class QTextDocument;
class AlteTheme; // Forward declare AlteTheme

struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
};

class AlteSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    AlteSyntaxHighlighter(QTextDocument *parent, AlteTheme *theme); // Modified constructor
    void loadRulesFromLanguageFile(const QString &filePath);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightingRule> highlightingRules;
    AlteTheme *m_theme; // Store the theme
};

#endif // ALTESYNTAXHIGHLIGHTER_H
