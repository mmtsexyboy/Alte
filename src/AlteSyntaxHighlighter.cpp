#include "AlteSyntaxHighlighter.h"
#include "AlteTheme.h" // Required for AlteTheme
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AlteSyntaxHighlighter::AlteSyntaxHighlighter(QTextDocument *parent, AlteTheme *theme)
    : QSyntaxHighlighter(parent), m_theme(theme) {
    if (!m_theme) {
        qWarning() << "AlteSyntaxHighlighter initialized with a null theme!";
    }
}

void AlteSyntaxHighlighter::loadRulesFromLanguageFile(const QString &filePath) {
    m_rules.clear(); // Use m_rules

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
    QJsonArray rulesArray = rootObj["highlighting_rules"].toArray();

    for (const QJsonValue &val : rulesArray) {
        QJsonObject ruleObj = val.toObject();
        QString syntaxType = ruleObj["type"].toString();
        QString styleKey = ruleObj["style_key"].toString(syntaxType); // Fallback to syntaxType if style_key missing

        // Map syntax type from language file to theme format key if necessary (as before)
        // This can be made more generic or data-driven.
        if (syntaxType == "keywords") styleKey = "keyword";
        else if (syntaxType == "line_comment") styleKey = "comment";
        else if (syntaxType == "strings_double_quotes" || syntaxType == "strings_single_quotes") styleKey = "string";
        else if (syntaxType == "numbers") styleKey = "number";
        // For "multi_line_comment", styleKey would ideally be "comment" or "block_comment"
        // The JSON should provide this directly via "style_key". If it's "multi_line_comment",
        // and style_key is "comment", it will use the comment theme.

        QTextCharFormat format = m_theme->getFormat(styleKey);

        if (syntaxType == "multi_line_comment" || syntaxType == "multi_line_string") {
            QString startPatternStr = ruleObj["start_pattern"].toString();
            QString endPatternStr = ruleObj["end_pattern"].toString();
            int stateId = ruleObj["state_id"].toInt(0);

            if (startPatternStr.isEmpty() || endPatternStr.isEmpty() || stateId == 0) {
                qWarning() << syntaxType << "rule is missing start_pattern, end_pattern, or has invalid state_id (0). Name:" << ruleObj["name"].toString();
                continue;
            }
            // For multi-line strings, QRegularExpression::DotMatchesEverythingOption might be important
            // if the string content itself can span newlines and the regex needs to match across them.
            // However, QSyntaxHighlighter processes block by block (line by line).
            // The state mechanism handles the "spanning newlines" part.
            // The patterns themselves usually match within a single line.
            // For example, """ might be a start/end pattern.
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (ruleObj.contains("options") && ruleObj["options"].toString() == "DotMatchesEverythingOption") {
                 options = QRegularExpression::DotMatchesEverythingOption;
            }

            m_rules.append(HighlightingRule(QRegularExpression(startPatternStr, options),
                                            QRegularExpression(endPatternStr, options),
                                            format, stateId, styleKey));
        } else if (ruleObj.contains("list")) { // Keywords
            QStringList keywords = ruleObj["list"].toVariant().toStringList();
            for (const QString &keyword : keywords) {
                m_rules.append(HighlightingRule(QRegularExpression(QString("\\b%1\\b").arg(keyword)), format, styleKey));
            }
        } else if (ruleObj.contains("start_delimiter") && ruleObj.contains("end_delimiter")) { // Strings
            QString start = QRegularExpression::escape(ruleObj["start_delimiter"].toString());
            QString end = QRegularExpression::escape(ruleObj["end_delimiter"].toString());
            m_rules.append(HighlightingRule(QRegularExpression(QString("%1[^%2\\n]*%2").arg(start).arg(end)), format, styleKey));
        } else if (ruleObj.contains("start_delimiter")) { // Line comments
            QString start = QRegularExpression::escape(ruleObj["start_delimiter"].toString());
            m_rules.append(HighlightingRule(QRegularExpression(QString("%1[^\n]*").arg(start)), format, styleKey));
        } else if (syntaxType == "number" && ruleObj.contains("pattern")) { // Number rule
            QString patternStr = ruleObj["pattern"].toString();
            if (!patternStr.isEmpty()) {
                 QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
                 if (ruleObj.contains("options") && ruleObj["options"].toString() == "CaseInsensitiveOption") {
                    // Example if numbers could have case-specific parts like '0xFF' vs '0XFF'
                    options = QRegularExpression::CaseInsensitiveOption;
                 }
                m_rules.append(HighlightingRule(QRegularExpression(patternStr, options), format, styleKey));
            } else {
                qWarning() << "Number rule is missing pattern. Name:" << ruleObj["name"].toString();
            }
        }
         // Add more rule parsing logic here as needed
    }

    qDebug() << "Loaded" << m_rules.count() << "rules from" << filePath;
    rehighlight();
}

void AlteSyntaxHighlighter::highlightBlock(const QString &text) {
    if (!m_theme) return;

    // --- Handle multi-line state continuation ---
    int previousStateId = previousBlockState();
    const HighlightingRule* activeMultiLineRule = nullptr;

    if (previousStateId > 0) { // Previous block was part of a multi-line construct
        activeMultiLineRule = findMultiLineRuleByStateId(previousStateId);
        if (activeMultiLineRule) {
            QRegularExpressionMatch endMatch = activeMultiLineRule->endPattern.match(text);
            int endIndex = endMatch.capturedStart();
            int endLength = endMatch.capturedLength();

            if (endIndex != -1) { // End pattern found in current block
                setFormat(0, endIndex + endLength, activeMultiLineRule->format);
                setCurrentBlockState(0); // Return to default state
            } else { // End pattern not found, whole block is part of the multi-line comment
                setFormat(0, text.length(), activeMultiLineRule->format);
                setCurrentBlockState(activeMultiLineRule->stateId);
                return; // Block fully processed as multi-line
            }
        } else {
             // Invalid previous state, reset
            setCurrentBlockState(0);
        }
    }

    // --- Process single-line rules and new multi-line starts ---
    // This part needs to be careful about rule order and overlaps.
    // A common strategy: apply multi-line first, then single-line for remaining parts.
    // Or, check single-line, but if a multi-line starts, it takes precedence for the rest of the block.

    int searchStartIndex = 0; // Current position in the text to start searching for rules

    while(searchStartIndex < text.length()) {
        int blockStateToSet = 0; // Default state for end of block
        int earliestMatchStart = text.length();
        const HighlightingRule* matchedRuleThisPass = nullptr;
        int matchedLength = 0;

        // Check for the start of any multi-line rule FIRST
        for (const HighlightingRule &rule : m_rules) {
            if (rule.type != RuleType::MultiLine) continue;

            QRegularExpressionMatch match = rule.pattern.match(text, searchStartIndex);
            if (match.hasMatch() && match.capturedStart() < earliestMatchStart) {
                earliestMatchStart = match.capturedStart();
                matchedRuleThisPass = &rule; // Potential multi-line start
                // Matched length for a multi-line start is just the start pattern itself initially
                // The actual highlighting will depend on finding the end or EOB
            }
        }

        // If a multi-line rule starts before any single-line rule found so far (or if no single yet)
        if (matchedRuleThisPass) { // A multi-line rule start was found
            int commentStartIndex = earliestMatchStart;
            QRegularExpressionMatch endMatch = matchedRuleThisPass->endPattern.match(text, commentStartIndex + matchedRuleThisPass->pattern.match(text, commentStartIndex).capturedLength());

            if (endMatch.hasMatch()) { // End is in the same block
                int commentEndIndex = endMatch.capturedStart() + endMatch.capturedLength();
                setFormat(commentStartIndex, commentEndIndex - commentStartIndex, matchedRuleThisPass->format);
                searchStartIndex = commentEndIndex; // Continue after this multi-line comment
                blockStateToSet = 0; // Back to default state
            } else { // End is not in this block
                setFormat(commentStartIndex, text.length() - commentStartIndex, matchedRuleThisPass->format);
                blockStateToSet = matchedRuleThisPass->stateId;
                searchStartIndex = text.length(); // Whole block consumed
            }
            setCurrentBlockState(blockStateToSet);
            if(searchStartIndex >= text.length()) continue; // Move to next block if fully consumed
        }

        // If no multi-line rule started, or we are past a completed multi-line segment, process single-line rules
        // Reset earliestMatch for single-line rules for the current searchStartIndex
        earliestMatchStart = text.length();
        const HighlightingRule* singleLineRuleToApply = nullptr;
        matchedLength = 0;

        for (const HighlightingRule &rule : m_rules) {
            if (rule.type != RuleType::SingleLine) continue;
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
             while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                if (match.capturedStart() >= searchStartIndex && match.capturedStart() < earliestMatchStart) {
                     earliestMatchStart = match.capturedStart();
                     singleLineRuleToApply = &rule;
                     matchedLength = match.capturedLength();
                } else if (match.capturedStart() > earliestMatchStart) {
                    // Since globalMatch iterates, if we've passed earliestMatchStart, later matches for this rule are past the candidate.
                    // However, other rules might match earlier.
                    break;
                }
            }
        }

        if (singleLineRuleToApply) {
            setFormat(earliestMatchStart, matchedLength, singleLineRuleToApply->format);
            searchStartIndex = earliestMatchStart + matchedLength;
        } else {
            // No more rules match in the remainder of the block
            searchStartIndex = text.length();
        }
         // If a multi-line comment was started in this loop, its state would have been set.
         // Otherwise, it remains 0 or what previousBlockState dictated if not changed.
        if (blockStateToSet != 0) setCurrentBlockState(blockStateToSet);
        else if (previousStateId == 0) setCurrentBlockState(0); // Ensure reset if no multi-line active
    }
}


const HighlightingRule* AlteSyntaxHighlighter::findMultiLineRuleByStateId(int stateId) const {
    for (const auto& rule : m_rules) {
        if (rule.type == RuleType::MultiLine && rule.stateId == stateId) {
            return &rule;
        }
    }
    return nullptr;
}
