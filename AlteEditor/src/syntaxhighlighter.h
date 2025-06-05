#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QString> // Added for m_languageName
#include <QJsonObject> // Added for parseTextCharFormat


class QTextDocument; // Forward declaration

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent, const QString &languageName); // Modified constructor

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;         // For start_delimiter or general pattern
        QTextCharFormat format;
        QRegularExpression endPattern;      // For end_delimiter of multi-line comments
        int stateId = 0;                // 0 means not a stateful rule (not a multi-line comment start). Used to identify the state.
        int formatCaptureGroup = 0;     // 0 for whole match, >0 for specific capture group (e.g., for definition names)
        // QString ruleType; // Optional: for debugging or more complex dispatch
    };
    QList<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression; // Kept for now, but less central
    QRegularExpression commentEndExpression;   // Kept for now, but less central

    void loadSyntaxDefinition(const QString &languageName);
    QTextCharFormat parseTextCharFormat(const QJsonObject &jsonFormatObject);

    // Helper function for multi-line comment processing
    // Returns the index in the text after the processed comment, or text.length() if it extends to EOB.
    // Modifies currentBlockState as needed.
    int matchMultiLineCommentAndFormat(const QString &text, const HighlightingRule &rule, int searchOffset, bool isContinuation);

    QString m_languageName; // Added to store the language name
};

#endif // SYNTAXHIGHLIGHTER_H
