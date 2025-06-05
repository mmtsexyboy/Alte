#include "AlteSyntaxHighlighter.h"
#include "AlteTheme.h" // Required for AlteTheme
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AlteSyntaxHighlighter::AlteSyntaxHighlighter(QTextDocument *parent, AlteTheme *theme)
    : QSyntaxHighlighter(parent), m_theme(theme) {
    // Constructor no longer initializes hardcoded formats.
    // It relies on m_theme being valid.
    if (!m_theme) {
        qWarning() << "AlteSyntaxHighlighter initialized with a null theme!";
        // Optionally, create a fallback AlteTheme instance here if essential
    }
}

void AlteSyntaxHighlighter::loadRulesFromLanguageFile(const QString &filePath) {
    highlightingRules.clear();

    if (filePath.isEmpty() || !m_theme) {
        if (!m_theme) qWarning() << "No theme set in highlighter, cannot apply formats.";
        if (filePath.isEmpty()) qInfo() << "No language file provided, clearing rules.";
        rehighlight();
        return;
    }

    QFile langFile(filePath);
    if (!langFile.exists()) {
        qWarning() << "Language file does not exist:" << filePath;
        rehighlight();
        return;
    }
    if (!langFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open language file:" << filePath;
        rehighlight();
        return;
    }

    QByteArray jsonData = langFile.readAll();
    langFile.close();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

    if (doc.isNull()) {
        qWarning() << "Failed to parse language JSON from" << filePath << ":" << error.errorString();
        rehighlight();
        return;
    }

    QJsonObject rootObj = doc.object();
    // QString languageName = rootObj["language_name"].toString(); // Could be used later

    QJsonArray rulesArray = rootObj["highlighting_rules"].toArray();
    for (const QJsonValue &val : rulesArray) {
        QJsonObject ruleObj = val.toObject();
        HighlightingRule currentRule;

        QString syntaxType = ruleObj["type"].toString(); // e.g., "keywords", "line_comment" from python.json

        // Map syntax type from language file to theme format key
        // This mapping logic can be expanded or made more configurable.
        QString themeFormatKey = syntaxType;
        if (syntaxType == "keywords") {
            themeFormatKey = "keyword";
        } else if (syntaxType == "line_comment") {
            themeFormatKey = "comment";
        } else if (syntaxType == "strings_double_quotes" || syntaxType == "strings_single_quotes") {
            themeFormatKey = "string";
        } else if (syntaxType == "numbers") { // Assuming you might add "numbers" to python.json
            themeFormatKey = "number";
        }
        // If syntaxType is "operator", "preprocessor", etc., it would map directly if keys are same.

        currentRule.format = m_theme->getFormat(themeFormatKey);

        if (ruleObj.contains("list")) { // For "keywords" type lists
            QStringList keywords = ruleObj["list"].toVariant().toStringList();
            foreach (const QString &keyword, keywords) {
                // Using \b for word boundaries is important for keywords
                currentRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(keyword));
                highlightingRules.append(currentRule);
            }
        } else if (ruleObj.contains("start_delimiter") && ruleObj.contains("end_delimiter")) { // For strings
            QString start = QRegularExpression::escape(ruleObj["start_delimiter"].toString());
            QString end = QRegularExpression::escape(ruleObj["end_delimiter"].toString());
            // Basic regex for strings: start_delimiter + anything_not_end_delimiter_or_newline + end_delimiter
            // Does not handle escaped delimiters within the string itself yet.
            currentRule.pattern = QRegularExpression(QString("%1[^%2\n]*%2").arg(start).arg(end));
            highlightingRules.append(currentRule);
        } else if (ruleObj.contains("start_delimiter")) { // For line comments
            QString start = QRegularExpression::escape(ruleObj["start_delimiter"].toString());
            // Matches start_delimiter to the end of the line
            currentRule.pattern = QRegularExpression(QString("%1[^\n]*").arg(start));
            highlightingRules.append(currentRule);
        }
        // Add more rule parsing logic here as needed (e.g., for multi-line comments, different regex patterns)
    }

    qDebug() << "Loaded" << highlightingRules.count() << "rules from" << filePath;
    rehighlight(); // Trigger re-highlighting with new rules
}

void AlteSyntaxHighlighter::highlightBlock(const QString &text) {
    if (!m_theme) return; // Don't highlight if no theme is set

    // Apply a default format for the whole block first, based on the theme's "text" color.
    // This ensures the base text color is consistent with the theme.
    // However, QSyntaxHighlighter applies formats on top of the QTextEdit's base format.
    // So, this line might not be strictly necessary if the QTextEdit's palette is already set correctly.
    // setFormat(0, text.length(), m_theme->getFormat("default"));


    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    // For multi-line comments/strings, setCurrentBlockState and previousBlockState() are needed.
}
