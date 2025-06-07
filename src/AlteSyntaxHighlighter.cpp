#include "AlteSyntaxHighlighter.h"
#include "AlteThemeManager.h"
#include <QTextDocument>
#include <QJsonArray>
#include <QDebug>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, AlteThemeManager *themeManager, const QString& languageName)
    : QSyntaxHighlighter(parent) {
    if (themeManager && !languageName.isEmpty()) {
        setCurrentLanguage(languageName, themeManager);
    } else {
        qWarning() << "SyntaxHighlighter: ThemeManager or languageName not provided. No rules loaded.";
    }
}

void SyntaxHighlighter::setCurrentLanguage(const QString& languageName, AlteThemeManager *themeManager) {
    m_highlightingRules.clear();
    if (!themeManager) {
        qWarning() << "SyntaxHighlighter::setCurrentLanguage: ThemeManager is null.";
        rehighlight();
        return;
    }
    if (languageName.isEmpty()){
        qWarning() << "SyntaxHighlighter::setCurrentLanguage: languageName is empty.";
        rehighlight();
        return;
    }
    loadRulesForLanguage(languageName, themeManager);
    rehighlight(); // Trigger re-highlighting of the entire document
}

QTextCharFormat SyntaxHighlighter::createFormatFromRule(const QJsonObject& ruleDetails,
                                                      const QJsonObject& themeColors, // Not used directly, themeManager is used
                                                      const QFont& defaultFont,
                                                      AlteThemeManager* themeManager) {
    QTextCharFormat format;
    QString colorNameRef = ruleDetails.value("color_ref").toString(); // Changed from "color"
    if (!colorNameRef.isEmpty()) {
        QColor color = themeManager->getColor(colorNameRef); // Use color_ref
        if (!color.isValid()) { // Fallback if color name is not in theme's "colors"
             qWarning() << "SyntaxHighlighter: Color reference '" << colorNameRef << "' is invalid or not found in theme colors. Using default text color.";
             color = themeManager->getColor("text", Qt::black); // Default to theme's text color
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
            QFont currentFont = format.font(); // Get font if already modified by bold/italic
            if (currentFont.pointSize() <= 0) { // If not set, use default
                 currentFont = defaultFont;
            }
            int newSize = currentFont.pointSize() + offset;
            if (newSize > 0) {
                currentFont.setPointSize(newSize);
                format.setFont(currentFont); // Apply the font with new size
            } else {
                qWarning() << "SyntaxHighlighter: Calculated font point size is not positive (" << newSize << "). Ignoring offset.";
            }
        }
    }
    return format;
}

void SyntaxHighlighter::loadRulesForLanguage(const QString& languageName, AlteThemeManager *themeManager) {
    qDebug() << ">>> loadRulesForLanguage V3 executing <<<";
    qDebug() << "SyntaxHighlighter::loadRulesForLanguage - Loading rules for language:" << languageName;
    m_highlightingRules.clear();
    QJsonObject langRules = themeManager->getSyntaxRulesForLanguage(languageName);
    qDebug() << "SyntaxHighlighter::loadRulesForLanguage - Received langRules from ThemeManager:" << langRules;

    if (langRules.isEmpty()) {
        qWarning() << "SyntaxHighlighter: No syntax rules found for language" << languageName << "(langRules object is empty).";
        return;
    }

    QFont documentFont;
    if (document()) {
        documentFont = document()->defaultFont();
    } else {
        documentFont = QApplication::font(); // Fallback, should ideally not happen
        qWarning() << "SyntaxHighlighter: QTextDocument is null during rule loading. Using application default font.";
    }

    // The themeColors parameter in createFormatFromRule is currently unused,
    // as themeManager->getColor() is preferred for direct lookup or hex parsing.
    // Passing an empty QJsonObject for now.
    QJsonObject dummyColors;

    // In the new JSON structure, langRules is an object like:
    // { "language_name": "Python", "file_extensions": [".py"], "highlighting_rules": [ {rule1}, {rule2} ] }
    // We need to iterate over the "highlighting_rules" array.

    if (!langRules.contains("highlighting_rules") || !langRules.value("highlighting_rules").isArray()) {
        qWarning() << "SyntaxHighlighter: 'highlighting_rules' array not found or not an array for language" << languageName << "Def:" << langRules;
        return;
    }
    QJsonArray rulesArray = langRules.value("highlighting_rules").toArray();

    for (const QJsonValue& ruleValue : rulesArray) {
        QJsonObject ruleDef = ruleValue.toObject();
        QString ruleName = ruleDef.value("name").toString("Unnamed Rule"); // For logging

        HighlightingRule baseRuleSetup;
        baseRuleSetup.format = createFormatFromRule(ruleDef, dummyColors, documentFont, themeManager);
        baseRuleSetup.isBlockRule = false; // Default

        QString ruleType = ruleDef.value("type").toString();

        if (ruleType.isEmpty()) {
            qWarning() << "SyntaxHighlighter: Rule" << ruleName << "is missing 'type' field. Def:" << ruleDef;
            continue;
        }

        if (ruleType == "keywords") {
            if (!ruleDef.contains("list")) {
                qWarning() << "SyntaxHighlighter: 'keywords' rule" << ruleName << "is missing 'list' field. Def:" << ruleDef;
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
                    qWarning() << "SyntaxHighlighter: Invalid regex from keyword in list" << val.toString() << "for rule" << ruleName;
                }
            }
        } else if (ruleType == "line_comment") {
            if (!ruleDef.contains("start_delimiter")) {
                qWarning() << "SyntaxHighlighter: 'line_comment' rule" << ruleName << "is missing 'start_delimiter' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule specificRule = baseRuleSetup;
            QString delimiter = ruleDef.value("start_delimiter").toString();
            if (!delimiter.isEmpty()) {
                specificRule.pattern = QRegularExpression(QRegularExpression::escape(delimiter) + ".*");
                if (specificRule.pattern.isValid()) {
                    m_highlightingRules.append(specificRule);
                } else {
                    qWarning() << "SyntaxHighlighter: Invalid regex from line_comment rule" << ruleName << "for delimiter" << delimiter;
                }
            } else {
                qWarning() << "SyntaxHighlighter: Empty delimiter for line_comment rule" << ruleName;
            }
        } else if (ruleType == "multi_line_string") {
            if (!ruleDef.contains("start_pattern")) {
                qWarning() << "SyntaxHighlighter: 'multi_line_string' rule" << ruleName << "is missing 'start_pattern' field. Def:" << ruleDef;
                continue;
            }
            if (!ruleDef.contains("end_pattern")) {
                qWarning() << "SyntaxHighlighter: 'multi_line_string' rule" << ruleName << "is missing 'end_pattern' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule blockRule = baseRuleSetup;
            blockRule.isBlockRule = true;
            blockRule.pattern = QRegularExpression(ruleDef.value("start_pattern").toString());
            blockRule.endPattern = QRegularExpression(ruleDef.value("end_pattern").toString());
            if (blockRule.pattern.isValid() && blockRule.endPattern.isValid()) {
                m_highlightingRules.append(blockRule);
            } else {
                qWarning() << "SyntaxHighlighter: Invalid regex for 'multi_line_string' rule" << ruleName
                           << ": Start:" << ruleDef.value("start_pattern").toString()
                           << "End:" << ruleDef.value("end_pattern").toString();
            }
        } else if (ruleType == "pattern") {
            if (!ruleDef.contains("pattern")) {
                qWarning() << "SyntaxHighlighter: 'pattern' rule" << ruleName << "is missing 'pattern' field. Def:" << ruleDef;
                continue;
            }
            HighlightingRule singlePatternRule = baseRuleSetup;
            QString patternStr = ruleDef.value("pattern").toString();
            if (patternStr.isEmpty()){
                 qWarning() << "SyntaxHighlighter: Empty pattern string for 'pattern' rule" << ruleName;
                 continue;
            }
            singlePatternRule.pattern = QRegularExpression(patternStr);
            if (singlePatternRule.pattern.isValid()) {
                m_highlightingRules.append(singlePatternRule);
            } else {
                qWarning() << "SyntaxHighlighter: Invalid regex for 'pattern' rule" << ruleName << ":" << patternStr;
            }
        } else if (ruleDef.contains("patterns")) { // Legacy path, if still needed
            // This can be kept for backward compatibility or removed if all JSONs are updated.
            // For now, let's assume it's similar to "keywords" with "list" but uses "patterns" key.
            qWarning() << "SyntaxHighlighter: Rule" << ruleName << "uses legacy 'patterns' key. Consider updating to 'list' under 'keywords' type.";
            if (!ruleDef.contains("patterns")) { // Should not happen if previous 'contains' is true
                 qWarning() << "SyntaxHighlighter: 'patterns' rule" << ruleName << "is missing 'patterns' field. Def:" << ruleDef;
                 continue;
            }
            QJsonArray patternsArray = ruleDef.value("patterns").toArray();
            for (const QJsonValue& val : patternsArray) {
                HighlightingRule specificRule = baseRuleSetup; // Copy format
                QString patternString = "\\b" + QRegularExpression::escape(val.toString()) + "\\b";
                specificRule.pattern = QRegularExpression(patternString);
                if (specificRule.pattern.isValid()) {
                    m_highlightingRules.append(specificRule);
                } else {
                    qWarning() << "SyntaxHighlighter: Invalid regex from legacy 'patterns' list" << val.toString() << "for rule" << ruleName;
                }
            }
        } else {
            // Rule type not recognized or other issue.
            // The ruleDef.value("block").toBool(false) was for a flatter structure.
            // Now, "multi_line_string" is the explicit type for blocks.
            if (!ruleName.startsWith("_comment_")) { // Don't warn for meta-comment keys in JSON
                 qWarning() << "SyntaxHighlighter: Rule" << ruleName << "has unknown type'" << ruleType << "' or is malformed. Def:" << ruleDef;
            }
        }
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Apply non-block rules first
    for (const HighlightingRule &rule : m_highlightingRules) {
        if (rule.isBlockRule) continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle block rules (multi-line comments, strings etc.)
    // This state machine is simplified and assumes block rules don't nest in complex ways
    // with other block rules. It processes one block type at a time per line.

    int blockRuleIndex = previousBlockState(); // 0 = no state, >0 means rule index + 1

    if (blockRuleIndex > 0) { // Resuming a block from previous line
        int ruleIdx = blockRuleIndex - 1;
        if (ruleIdx < m_highlightingRules.size() && m_highlightingRules[ruleIdx].isBlockRule) {
            const HighlightingRule& currentBlockRule = m_highlightingRules[ruleIdx];
            QRegularExpressionMatch endMatch = currentBlockRule.endPattern.match(text, 0);
            int endIndex = endMatch.capturedStart();
            int length;

            if (endIndex == -1) { // Block does not end on this line
                setCurrentBlockState(blockRuleIndex); // Continue this block state
                length = text.length();
                setFormat(0, length, currentBlockRule.format);
                return; // Entire line is part of this block
            } else { // Block ends on this line
                length = endIndex + endMatch.capturedLength();
                setFormat(0, length, currentBlockRule.format);
                setCurrentBlockState(0); // Block ended
                // Now, search for new blocks *after* this one ended
                // This recursive call is one way, or loop with adjusted startIndex
                // For simplicity, we'll let the next loop handle new blocks.
                // highlightBlock(text.mid(length)); // This is not how QSyntaxHighlighter works.
                // Instead, we need to continue scanning from 'length'.
                // The logic below will scan from 0, which is fine if blocks don't overlap.
                // A more robust solution might need a loop here that advances a startIndex.
            }
        } else {
             setCurrentBlockState(0); // Invalid previous state
        }
    }

    // Search for new block starts
    for (int i = 0; i < m_highlightingRules.size(); ++i) {
        const HighlightingRule &rule = m_highlightingRules[i];
        if (!rule.isBlockRule) continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch startMatch = matchIterator.next();

            // Check if this match is already formatted by a higher-priority rule or a completed block
            // This is a simplification; proper handling of overlapping rules is complex.
            // For now, we assume if currentBlockState became 0, we can format.
            // A more robust check would involve checking formats at startMatch.capturedStart().
            if (currentBlockState() != 0 && startMatch.capturedStart() == 0) {
                 // This part of the line is already consumed by a continued block.
                 // This check is imperfect. A better way is to manage startIndex.
                continue;
            }


            QRegularExpressionMatch endMatch = rule.endPattern.match(text, startMatch.capturedEnd());
            int length;
            if (endMatch.capturedStart() == -1) { // Block starts here and does not end on this line
                setCurrentBlockState(i + 1); // Store (rule index + 1) as block state
                length = text.length() - startMatch.capturedStart();
                setFormat(startMatch.capturedStart(), length, rule.format);
                return; // Rest of the line is this block
            } else { // Block starts and ends on this line
                length = endMatch.capturedEnd() - startMatch.capturedStart();
                setFormat(startMatch.capturedStart(), length, rule.format);
                // Continue searching for more blocks after this one on the same line
                // This basic loop doesn't handle that well. It will find subsequent blocks
                // but might re-apply over earlier block matches if not careful.
                // For now, let's assume one block type takes precedence or they don't overlap this way.
            }
        }
    }
}
