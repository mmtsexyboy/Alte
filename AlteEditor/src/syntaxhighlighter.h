#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>      // Using QVector as it's common, can revert to QList if preferred
#include <QJsonObject>
#include <QFont>        // For font point size adjustments

class AlteThemeManager; // Forward declaration
class QTextDocument;

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent, AlteThemeManager *themeManager, const QString& languageName);
    void setCurrentLanguage(const QString& languageName, AlteThemeManager *themeManager);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        bool isBlockRule; // For multi-line comments/strings
        QRegularExpression endPattern; // Only for block rules
    };
    QVector<HighlightingRule> m_highlightingRules; // Changed to QVector and renamed

    // Removed C++ specific format members
    // QRegularExpression commentStartExpression; // Will be part of HighlightingRule if needed for block comments
    // QRegularExpression commentEndExpression;   // Same as above

    void loadRulesForLanguage(const QString& languageName, AlteThemeManager *themeManager);
    QTextCharFormat createFormatFromRule(const QJsonObject& ruleDetails,
                                         const QJsonObject& themeColors, // General theme colors
                                         const QFont& defaultFont,
                                         AlteThemeManager* themeManager); // To use themeManager->getColor
};

#endif // SYNTAXHIGHLIGHTER_H
