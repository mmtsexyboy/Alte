#include "AlteSyntaxHighlighter.h"
#include "AlteThemeManager.h"
#include <QTextDocument>
#include <QJsonArray>
#include <QDebug>

AlteSyntaxHighlighter::AlteSyntaxHighlighter(QTextDocument *parent, AlteThemeManager *themeManager, const QString& languageName)
    : QSyntaxHighlighter(parent) {
    if (themeManager && !languageName.isEmpty()) {
        setCurrentLanguage(languageName, themeManager);
    } else {
        qWarning() << "SyntaxHighlighter: ThemeManager or languageName not provided. No rules loaded.";
    }
}

void AlteSyntaxHighlighter::setCurrentLanguage(const QString& languageName, AlteThemeManager *themeManager) {
    m_highlightingRules.clear();
    if (!themeManager) {
        qWarning() << "AlteSyntaxHighlighter::setCurrentLanguage: ThemeManager is null.";
        rehighlight();
        return;
    }
    if (languageName.isEmpty()){
        qWarning() << "AlteSyntaxHighlighter::setCurrentLanguage: languageName is empty.";
        rehighlight();
        return;
    }
    loadRulesForLanguage(languageName, themeManager);
    rehighlight();
}

QTextCharFormat AlteSyntaxHighlighter::createFormatFromRule(const QJsonObject& ruleDetails,
                                                      const QJsonObject& themeColors,
                                                      const QFont& defaultFont,
                                                      AlteThemeManager* themeManager) {
    QTextCharFormat format;
    QString colorNameRef = ruleDetails.value("color_ref").toString();
    if (!colorNameRef.isEmpty()) {
        QColor color = themeManager->getColor(colorNameRef);
        if (!color.isValid()) {
             qWarning() << "AlteSyntaxHighlighter: Color reference '" << colorNameRef << "' is invalid or not found in theme colors. Using default text color.";
             color = themeManager->getColor("text", Qt::black);
        }
        format.setForeground(color);
    }

    if (ruleDetails.value("bold").toBool(false)) {
        format.setFontWeight(QFont::Bold);
    }
    if (ruleDetails.value("italic").toBool(false)) {
        format.setFontItalic(true);
    }

    if (ruleDetails.contains("fontPointSizeOffset")) {
        int offset = ruleDetails.value("fontPointSizeOffset").toInt(0);
        if (offset != 0) {
            QFont currentFont = format.font();
            if (currentFont.pointSize() <= 0) {
                 currentFont = defaultFont;
            }
            int newSize = currentFont.pointSize() + offset;
            if (newSize > 0) {
                currentFont.setPointSize(newSize);
                format.setFont(currentFont); // Apply the font with new size
            } else {
                qWarning() << "AlteSyntaxHighlighter: Calculated font point size is not positive (" << newSize << "). Ignoring offset.";
            }
        }
    }
    return format;
}

void AlteSyntaxHighlighter::loadRulesForLanguage(const QString& languageName, AlteThemeManager *themeManager) {
    qDebug() << ">>> loadRulesForLanguage V3 executing <<<";
    qDebug() << "AlteSyntaxHighlighter::loadRulesForLanguage - Loading rules for language:" << languageName;
    m_highlightingRules.clear();
    QJsonObject langRules = themeManager->getSyntaxRulesForLanguage(languageName);
    qDebug() << "AlteSyntaxHighlighter::loadRulesForLanguage - Received langRules from ThemeManager:" << langRules;

    if (langRules.isEmpty()) {
        qWarning() << "AlteSyntaxHighlighter: No syntax rules found for language" << languageName << "(langRules object is empty).";
        return;
    }

    QFont documentFont;
    if (document()) {
        documentFont = document()->defaultFont();
    } else {
        documentFont = QApplication::font();
        qWarning() << "AlteSyntaxHighlighter: QTextDocument is null during rule loading. Using application default font.";
    }

    QJsonObject dummyColors;

    if (!langRules.contains("highlighting_rules") || !langRules.value("highlighting_rules").isArray()) {
        qWarning() << "AlteSyntaxHighlighter: 'highlighting_rules' array not found or not an array for language" << languageName << "Def:" << langRules;
        return;
    }
    QJsonArray rulesArray = langRules.value("highlighting_rules").toArray();

    for (const QJsonValue& ruleValue : rulesArray) {
        QJsonObject ruleDef = ruleValue.toObject();
        QString ruleName = ruleDef.value("name").toString("Unnamed Rule");

        HighlightingRule baseRuleSetup;
        baseRuleSetup.format = createFormatFromRule(ruleDef, dummyColors, documentFont, themeManager);
        baseRuleSetup.isBlockRule = false;

        QString ruleType = ruleDef.value("type").toString();

        if (ruleType.isEmpty()) {
            qWarning() << "AlteSyntaxHighlighter: Rule" << ruleName << "is missing 'type' field. Def:" << ruleDef;
            continue;
        }

        if (ruleType == "keywords") {
            if (!ruleDef.contains("list")) {
                qWarning() << "AlteSyntaxHighlighter: 'keywords' rule" << ruleName << "is missing 'list' field. Def:" << ruleDef;
                continue;
            }
            QJsonArray patternsArray = ruleDef.value("list").toArray();
            for (const QJsonValue& val : patternsArray) {
                HighlightingRule specificRule = baseRuleSetup;
                QString patternString = "\\b" + QRegularExpression::escape(val.toString()) + "\\b";
                specificRule.pattern = QRegularExpression(patternString);
                if (specificRule.pattern.isValid()) {
                    m_highlightingRules.append(specificRule);
                } else {
                    qWarning() << "AlteSyntaxHighlighter: Invalid regex from keyword in list" << val.toString() << "for rule" << ruleName;
                }
            }
        } else if (ruleType == "line_comment") {
            if (!ruleDef.contains("start_delimiter")) {
                qWarning() << "AlteSyntaxHighlighter: 'line_comment' rule" << ruleName << "is missing 'start_delimiter' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule specificRule = baseRuleSetup;
            QString delimiter = ruleDef.value("start_delimiter").toString();
            if (!delimiter.isEmpty()) {
                specificRule.pattern = QRegularExpression(QRegularExpression::escape(delimiter) + ".*");
                if (specificRule.pattern.isValid()) {
                    m_highlightingRules.append(specificRule);
                } else {
                    qWarning() << "AlteSyntaxHighlighter: Invalid regex from line_comment rule" << ruleName << "for delimiter" << delimiter;
                }
            } else {
                qWarning() << "AlteSyntaxHighlighter: Empty delimiter for line_comment rule" << ruleName;
            }
        } else if (ruleType == "multi_line_string") {
            if (!ruleDef.contains("start_pattern")) {
                qWarning() << "AlteSyntaxHighlighter: 'multi_line_string' rule" << ruleName << "is missing 'start_pattern' field. Def:" << ruleDef;
                continue;
            }
            if (!ruleDef.contains("end_pattern")) {
                qWarning() << "AlteSyntaxHighlighter: 'multi_line_string' rule" << ruleName << "is missing 'end_pattern' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule blockRule = baseRuleSetup;
            blockRule.isBlockRule = true;
            blockRule.pattern = QRegularExpression(ruleDef.value("start_pattern").toString());
            blockRule.endPattern = QRegularExpression(ruleDef.value("end_pattern").toString());
            if (blockRule.pattern.isValid() && blockRule.endPattern.isValid()) {
                m_highlightingRules.append(blockRule);
            } else {
                qWarning() << "AlteSyntaxHighlighter: Invalid regex for 'multi_line_string' rule" << ruleName
                           << ": Start:" << ruleDef.value("start_pattern").toString()
                           << "End:" << ruleDef.value("end_pattern").toString();
            }
        } else if (ruleType == "pattern") {
            if (!ruleDef.contains("pattern")) {
                qWarning() << "AlteSyntaxHighlighter: 'pattern' rule" << ruleName << "is missing 'pattern' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule singlePatternRule = baseRuleSetup;
            QString patternStr = ruleDef.value("pattern").toString();
            if (patternStr.isEmpty()){
                 qWarning() << "AlteSyntaxHighlighter: Empty pattern string for 'pattern' rule" << ruleName;
                 continue;
            }
            singlePatternRule.pattern = QRegularExpression(patternStr);
            if (singlePatternRule.pattern.isValid()) {
                m_highlightingRules.append(singlePatternRule);
            } else {
                qWarning() << "AlteSyntaxHighlighter: Invalid regex for 'pattern' rule" << ruleName << ":" << patternStr;
            }
        } else if (ruleDef.contains("patterns")) { // Legacy path, if still needed
            // This can be kept for backward compatibility or removed if all JSONs are updated.
            // For now, let's assume it's similar to "keywords" with "list" but uses "patterns" key.
            qWarning() << "AlteSyntaxHighlighter: Rule" << ruleName << "uses legacy 'patterns' key. Consider updating to 'list' under 'keywords' type.";
            if (!ruleDef.contains("patterns")) { // Should not happen if previous 'contains' is true
                 qWarning() << "AlteSyntaxHighlighter: 'patterns' rule" << ruleName << "is missing 'patterns' field. Def:" << ruleDef;
                 continue;
            }
            QJsonArray patternsArray = ruleDef.value("patterns").toArray();
            for (const QJsonValue& val : patternsArray) {
                HighlightingRule specificRule = baseRuleSetup;
                QString patternString = "\\b" + QRegularExpression::escape(val.toString()) + "\\b";
                specificRule.pattern = QRegularExpression(patternString);
                if (specificRule.pattern.isValid()) {
                    m_highlightingRules.append(specificRule);
                } else {
                    qWarning() << "AlteSyntaxHighlighter: Invalid regex from legacy 'patterns' list" << val.toString() << "for rule" << ruleName;
                }
            }
        } else {
            if (!ruleName.startsWith("_comment_")) {
                 qWarning() << "AlteSyntaxHighlighter: Rule" << ruleName << "has unknown type'" << ruleType << "' or is malformed. Def:" << ruleDef;
            }
        }
    }
}

void AlteSyntaxHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : m_highlightingRules) {
        if (rule.isBlockRule) continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    int blockRuleIndex = previousBlockState();

    if (blockRuleIndex > 0) {
        int ruleIdx = blockRuleIndex - 1;
        if (ruleIdx < m_highlightingRules.size() && m_highlightingRules[ruleIdx].isBlockRule) {
            const HighlightingRule& currentBlockRule = m_highlightingRules[ruleIdx];
            QRegularExpressionMatch endMatch = currentBlockRule.endPattern.match(text, 0);
            int endIndex = endMatch.capturedStart();
            int length;

            if (endIndex == -1) {
                setCurrentBlockState(blockRuleIndex);
                length = text.length();
                setFormat(0, length, currentBlockRule.format);
                return;
            } else {
                length = endIndex + endMatch.capturedLength();
                setFormat(0, length, currentBlockRule.format);
                setCurrentBlockState(0);
            }
        } else {
             setCurrentBlockState(0);
        }
    }

    for (int i = 0; i < m_highlightingRules.size(); ++i) {
        const HighlightingRule &rule = m_highlightingRules[i];
        if (!rule.isBlockRule) continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch startMatch = matchIterator.next();

            if (currentBlockState() != 0 && startMatch.capturedStart() == 0) {
                continue;
            }


            QRegularExpressionMatch endMatch = rule.endPattern.match(text, startMatch.capturedEnd());
            int length;
            if (endMatch.capturedStart() == -1) {
                setCurrentBlockState(i + 1);
                length = text.length() - startMatch.capturedStart();
                setFormat(startMatch.capturedStart(), length, rule.format);
                return;
            } else {
                length = endMatch.capturedEnd() - startMatch.capturedStart();
                setFormat(startMatch.capturedStart(), length, rule.format);
            }
        }
    }
}
