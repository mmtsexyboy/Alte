#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QJsonObject>
#include <QFont>

class AlteThemeManager;
class QTextDocument;

class AlteSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    AlteSyntaxHighlighter(QTextDocument *parent, AlteThemeManager *themeManager, const QString& languageName);
    void setCurrentLanguage(const QString& languageName, AlteThemeManager *themeManager);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        bool isBlockRule;
        QRegularExpression endPattern;
    };
    QVector<HighlightingRule> m_highlightingRules;

    void loadRulesForLanguage(const QString& languageName, AlteThemeManager *themeManager);
    QTextCharFormat createFormatFromRule(const QJsonObject& ruleDetails,
                                         const QJsonObject& themeColors,
                                         const QFont& defaultFont,
                                         AlteThemeManager* themeManager);
};

#endif // SYNTAXHIGHLIGHTER_H
