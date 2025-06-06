#ifndef ALTESYNTAXHIGHLIGHTER_H
#define ALTESYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

// Forward declaration
class QTextDocument;
class AlteTheme; // Forward declare AlteTheme

enum class RuleType {
    SingleLine,
    MultiLine
};

struct HighlightingRule {
    RuleType type = RuleType::SingleLine;
    QRegularExpression pattern;      // Used for single-line rules, or start_pattern for multi-line
    QTextCharFormat format;
    // For multi-line rules:
    QRegularExpression endPattern;
    int stateId = 0; // 0 is default/no state, positive integers are custom states
    QString styleKey; // To re-fetch format if needed, or store format directly

    // Constructor for single-line rules
    HighlightingRule(const QRegularExpression& p, const QTextCharFormat& f, const QString& sk = "")
        : type(RuleType::SingleLine), pattern(p), format(f), styleKey(sk) {}

    // Constructor for multi-line rules
    HighlightingRule(const QRegularExpression& start_p, const QRegularExpression& end_p,
                     const QTextCharFormat& f, int s_id, const QString& sk)
        : type(RuleType::MultiLine), pattern(start_p), endPattern(end_p),
          format(f), stateId(s_id), styleKey(sk) {}

    HighlightingRule() = default; // Allow default construction for QVector
};

class AlteSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    AlteSyntaxHighlighter(QTextDocument *parent, AlteTheme *theme);
    void loadRulesFromLanguageFile(const QString &filePath);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightingRule> m_rules; // Combined rules
    AlteTheme *m_theme;
    // Helper to find a multi-line rule by its state ID
    const HighlightingRule* findMultiLineRuleByStateId(int stateId) const;
};

#endif // ALTESYNTAXHIGHLIGHTER_H
