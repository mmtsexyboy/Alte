#include "syntaxhighlighter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QDebug> // For qWarning()
#include <QFont>  // For QFont::Bold etc.

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent, const QString &languageName)
    : QSyntaxHighlighter(parent), m_languageName(languageName) {
    // m_languageName is initialized in the member initializer list
    if (!m_languageName.isEmpty()) {
        loadSyntaxDefinition(m_languageName);
    }
    // All C++ specific rules are removed from here.
    // commentStartExpression and commentEndExpression are kept as members for now,
    // but not initialized here unless a default/fallback is desired.
    // For JSON-driven highlighting, they would be populated by loadSyntaxDefinition
    // if the JSON contains rules for multi-line comments.
}

void SyntaxHighlighter::loadSyntaxDefinition(const QString &languageName) {
    // Clear any existing rules, in case this is called multiple times (e.g. language change)
    highlightingRules.clear();

    QString filePath = QString("resources/syntax/%1.json").arg(languageName);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { // Added QIODevice::Text
        qWarning() << "Cannot open syntax file:" << filePath << file.errorString();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing JSON syntax file:" << filePath << "Error:" << parseError.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "JSON syntax file is not an object:" << filePath;
        return;
    }

    QJsonObject rootObject = jsonDoc.object();
    // Language name and file extensions are not used by SyntaxHighlighter itself,
    // but can be useful for managing multiple syntax files.
    // We are interested in "highlighting_rules".

    if (!rootObject.contains("highlighting_rules") || !rootObject["highlighting_rules"].isArray()) {
        qWarning() << "JSON syntax file does not contain 'highlighting_rules' array:" << filePath;
        return;
    }

    QJsonArray rulesArray = rootObject["highlighting_rules"].toArray();

    for (const QJsonValue &value : rulesArray) {
        if (!value.isObject()) {
            qWarning() << "Invalid rule entry (not an object) in:" << filePath;
            continue;
        }
        QJsonObject ruleObject = value.toObject();
        HighlightingRule currentRule;

        if (!ruleObject.contains("type") || !ruleObject["type"].isString() ||
            !ruleObject.contains("format") || !ruleObject["format"].isObject()) {
            qWarning() << "Rule missing 'type' or 'format' in:" << filePath;
            continue;
        }

        QString type = ruleObject["type"].toString();
        QJsonObject formatObject = ruleObject["format"].toObject();
        currentRule.format = parseTextCharFormat(formatObject);

        // Common properties for rules with a single pattern
        if (ruleObject.contains("pattern") && ruleObject["pattern"].isString()) {
            currentRule.pattern = QRegularExpression(ruleObject["pattern"].toString());
            if (!currentRule.pattern.isValid()) {
                 qWarning() << "Invalid regex pattern provided for rule type" << type << "in:" << filePath << ruleObject["pattern"].toString();
                 continue; // Skip this rule
            }
        }
        if (ruleObject.contains("format_capture_group") && ruleObject["format_capture_group"].isDouble()) { // isDouble for integer numbers
            currentRule.formatCaptureGroup = static_cast<int>(ruleObject["format_capture_group"].toDouble());
        }


        if (type == "keywords") {
            if (!ruleObject.contains("list") || !ruleObject["list"].isArray()) {
                 qWarning() << "Keywords rule missing 'list' array in:" << filePath;
                 continue;
            }
            QJsonArray keywordList = ruleObject["list"].toArray();
            for (const QJsonValue &keywordVal : keywordList) {
                if (!keywordVal.isString()) {
                    qWarning() << "Invalid keyword (not a string) in 'list' for keywords rule:" << filePath;
                    continue;
                }
                HighlightingRule keywordSpecificRule;
                keywordSpecificRule.format = currentRule.format; // Base format for the group
                QString keyword = keywordVal.toString();
                keywordSpecificRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(QRegularExpression::escape(keyword)));
                highlightingRules.append(keywordSpecificRule);
            }
        } else if (type == "line_comment") {
            if (!ruleObject.contains("start_delimiter") || !ruleObject["start_delimiter"].isString()) {
                qWarning() << "Line comment rule missing 'start_delimiter' string in:" << filePath;
                continue;
            }
            QString startDelimiter = ruleObject["start_delimiter"].toString();
            currentRule.pattern = QRegularExpression(QString("%1[^\n]*").arg(QRegularExpression::escape(startDelimiter)));
            highlightingRules.append(currentRule);
        } else if (type == "multi_line_comment") {
            if (!ruleObject.contains("start_delimiter") || !ruleObject["start_delimiter"].isString() ||
                !ruleObject.contains("end_delimiter") || !ruleObject["end_delimiter"].isString() ||
                !ruleObject.contains("state_id") || !ruleObject["state_id"].isDouble()) { // isDouble for integer
                qWarning() << "Multi-line comment rule missing 'start_delimiter', 'end_delimiter', or 'state_id' in:" << filePath;
                continue;
            }
            currentRule.pattern = QRegularExpression(ruleObject["start_delimiter"].toString()); // This is the start pattern
            currentRule.endPattern = QRegularExpression(ruleObject["end_delimiter"].toString());
            currentRule.stateId = static_cast<int>(ruleObject["state_id"].toDouble());
            if (currentRule.stateId == 0) {
                 qWarning() << "Multi-line comment rule 'state_id' cannot be 0 in:" << filePath;
                 continue;
            }
            if (!currentRule.pattern.isValid() || !currentRule.endPattern.isValid()){
                qWarning() << "Invalid start or end regex pattern for multi_line_comment in:" << filePath;
                continue;
            }
            highlightingRules.append(currentRule);
        } else if (type == "strings_double_quotes" || type == "strings_single_quotes") { // Consolidate string types if pattern is given
            // This was handled by generic "pattern_rule" in the provided python.json example
            // If "start_delimiter" is used, it needs specific handling like before.
            // For now, assuming these are covered by "pattern_rule" if python.json is the guide.
            // If specific "start_delimiter" logic for strings is still needed:
            // if (ruleObject.contains("start_delimiter") && ruleObject["start_delimiter"].isString()) {
            //     QString delimiter = ruleObject["start_delimiter"].toString();
            //     currentRule.pattern = QRegularExpression(QString("%1([^%1\\\\]|\\\\.)*%1").arg(QRegularExpression::escape(delimiter)));
            //     highlightingRules.append(currentRule);
            // } else {
            //      qWarning() << type << " rule type expects a 'start_delimiter' or should be a 'pattern_rule'.";
            // }
            // Based on python.json, strings are defined with "pattern_rule", so this explicit type might be redundant
            // or needs to be more clearly defined if it offers something "pattern_rule" doesn't.
            // For now, let's assume "pattern_rule" is preferred for flexibility.
            // If the "pattern" was already parsed, and it's valid, it can be added.
            if (currentRule.pattern.isValid()) {
                 highlightingRules.append(currentRule);
            } else if (type == "strings_double_quotes" || type == "strings_single_quotes") {
                 // if pattern was not in the ruleObject directly for these types, it's an issue.
                 qWarning() << type << " rule type expects a 'pattern' or specific delimiters not yet fully handled here without 'pattern'.";
            }

        } else if (type == "number" || type == "definition_name" || type == "pattern_rule") {
            // These types rely on "pattern" being present in ruleObject, which was parsed above.
            // "format_capture_group" was also parsed if present.
            if (!currentRule.pattern.isValid()) { // Check if pattern was actually set and valid
                qWarning() << "Rule type" << type << "requires a valid 'pattern' in:" << filePath;
                continue;
            }
            highlightingRules.append(currentRule);
        } else {
            qWarning() << "Unsupported rule type or missing required fields for type:" << type << "in:" << filePath;
        }
    }
}

QTextCharFormat SyntaxHighlighter::parseTextCharFormat(const QJsonObject &jsonFormatObject) {
    QTextCharFormat format;
    if (jsonFormatObject.contains("color") && jsonFormatObject["color"].isString()) {
        format.setForeground(QColor(jsonFormatObject["color"].toString()));
    }
    if (jsonFormatObject.contains("font_weight") && jsonFormatObject["font_weight"].isString()) {
        if (jsonFormatObject["font_weight"].toString().toLower() == "bold") {
            format.setFontWeight(QFont::Bold);
        } else if (jsonFormatObject["font_weight"].toString().toLower() == "normal") {
            format.setFontWeight(QFont::Normal);
        } // Add other weights as needed e.g. QFont::DemiBold etc.
    }
    if (jsonFormatObject.contains("font_style") && jsonFormatObject["font_style"].isString()) {
        if (jsonFormatObject["font_style"].toString().toLower() == "italic") {
            format.setFontItalic(true);
        }
    }
    // Example for other properties
    // if (jsonFormatObject.contains("background_color") && jsonFormatObject["background_color"].isString()) {
    //     format.setBackground(QColor(jsonFormatObject["background_color"].toString()));
    // }
    return format;
}

// Helper function for multi-line comment processing
int SyntaxHighlighter::matchMultiLineCommentAndFormat(const QString &text, const HighlightingRule &rule, int searchOffset, bool isContinuation) {
    QRegularExpressionMatch match;

    // Effective search start is always the passed searchOffset for this specific invocation
    // The caller (highlightBlock) manages advancing currentPosition overall.

    if (isContinuation) {
        // Already in this comment state, look for the end delimiter from searchOffset
        match = rule.endPattern.match(text, searchOffset);
        if (match.hasMatch()) {
            // Comment ends in this block
            setFormat(searchOffset, (match.capturedStart() + match.capturedLength()) - searchOffset, rule.format);
            setCurrentBlockState(0); // Reset state
            return match.capturedStart() + match.capturedLength(); // Return position after the comment
        } else {
            // Comment continues to the end of the block
            setFormat(searchOffset, text.length() - searchOffset, rule.format);
            setCurrentBlockState(rule.stateId); // Maintain state
            return text.length(); // Consumed to end of block
        }
    } else {
        // Not a continuation, look for a new start of this comment type at searchOffset
        // We only care about matches that start *exactly* at searchOffset to ensure
        // the earliest rule at a given position is processed.
        match = rule.pattern.match(text, searchOffset);
        if (match.hasMatch() && match.capturedStart() == searchOffset) {
            int commentActualStartOffset = match.capturedStart();
            // Check if it also ends in this block
            QRegularExpressionMatch endMatch = rule.endPattern.match(text, commentActualStartOffset + match.capturedLength());
            if (endMatch.hasMatch()) {
                // Comment starts and ends in this block
                setFormat(commentActualStartOffset, (endMatch.capturedStart() + endMatch.capturedLength()) - commentActualStartOffset, rule.format);
                setCurrentBlockState(0); // Reset state
                return endMatch.capturedStart() + endMatch.capturedLength(); // Return position after the comment
            } else {
                // Comment starts but does not end in this block
                setFormat(commentActualStartOffset, text.length() - commentActualStartOffset, rule.format);
                setCurrentBlockState(rule.stateId); // Set state
                return text.length(); // Consumed to end of block
            }
        }
    }
    // If no match occurred (for a new comment starting exactly at searchOffset, or an end delimiter for a continuation)
    return searchOffset; // Indicate no match by this rule at this position, caller should advance.
}


void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Pass 1: Apply all non-stateful rules (those with stateId == 0)
    for (const HighlightingRule &rule : highlightingRules) {
        if (rule.stateId != 0) { // Skip multi-line comment rules for this pass
            continue;
        }
        if (!rule.pattern.isValid()) continue;

        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            if (rule.formatCaptureGroup > 0 && match.lastCapturedIndex() >= rule.formatCaptureGroup) {
                setFormat(match.capturedStart(rule.formatCaptureGroup), match.capturedLength(rule.formatCaptureGroup), rule.format);
            } else if (rule.formatCaptureGroup == 0) { // Default to whole match
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

    int currentPosition = 0;
    int previousSt = previousBlockState();

    if (previousSt != 0) { // Check if continuing a multi-line comment
        bool stateRuleFound = false;
        for (const HighlightingRule &rule : highlightingRules) {
            if (rule.stateId == previousSt) { // Find the rule that matches the previous state
                currentPosition = matchMultiLineCommentAndFormat(text, rule, 0, true); // Start search from beginning of block for continuation
                stateRuleFound = true;
                break;
            }
        }
        if (!stateRuleFound) {
            // This implies an invalid state from previous block or missing rule definition.
            // Reset state to be safe.
             qWarning() << "SyntaxHighlighter: Invalid previous block state or missing rule for state" << previousSt;
             setCurrentBlockState(0); // Reset state if rule for previous state not found
        }
        // If the continued comment consumed the whole block, currentBlockState() will be non-zero.
        if (currentBlockState() != 0 && currentPosition >= text.length()) {
            return; // Finished processing this block
        }
        // If the comment ended, currentBlockState() is 0, and currentPosition is where it ended.
        // If currentPosition is 0 here, it means the continuation rule didn't find an end pattern from the start.
        // This implies the whole block is part of the comment.
        if (currentPosition == 0 && currentBlockState() != 0) { // Rule was applied, consumed block
             return;
        }

    } else {
        setCurrentBlockState(0); // Start with no active multi-line comment state
    }

    // Pass 2: Scan for starts of multi-line comments from currentPosition
    // This loop continues as long as we make progress (currentPosition advances)
    // or a multi-line comment consumes the rest of the block.
    while (currentPosition < text.length()) {
        const HighlightingRule* nextRuleToProcess = nullptr;
        int earliestMatchPosition = text.length(); // Initialize to end of text

        // Find the multi-line comment rule that starts earliest from currentPosition
        for (const HighlightingRule &rule : highlightingRules) {
            if (rule.stateId == 0) { // Only interested in stateful (multi-line comment start) rules
                continue;
            }
            if (!rule.pattern.isValid()) continue;

            QRegularExpressionMatch match = rule.pattern.match(text, currentPosition);
            if (match.hasMatch() && match.capturedStart() < earliestMatchPosition) {
                earliestMatchPosition = match.capturedStart();
                nextRuleToProcess = &rule;
            }
        }

        if (nextRuleToProcess) {
            // A multi-line comment rule was found starting at earliestMatchPosition.
            // Any text between currentPosition and earliestMatchPosition is unstyled by multi-line comments (already styled by Pass 1).
            // Process this found comment.
            // The search for this new comment must start at its actual beginning (earliestMatchPosition).
            int nextPositionAfterComment = matchMultiLineCommentAndFormat(text, *nextRuleToProcess, earliestMatchPosition, false);

            if (currentBlockState() != 0) {
                // The comment started (or continued if somehow it was a 0-length match that turned stateful)
                // and consumed the rest of the block.
                return; // Stop processing this block
            } else {
                // The comment started and ended within the block.
                // Or, no match was made by matchMultiLineCommentAndFormat (should not happen if nextRuleToProcess is not null)
                // Advance currentPosition to after the processed comment.
                currentPosition = nextPositionAfterComment;
                // If nextPositionAfterComment didn't advance from earliestMatchPosition, it means the rule didn't "take".
                // This could happen if matchMultiLineCommentAndFormat returns searchOffset.
                // To prevent infinite loop, ensure progress:
                if (currentPosition <= earliestMatchPosition && earliestMatchPosition < text.length()) { // ensure not stuck
                    currentPosition = earliestMatchPosition + 1; // Force advancement if no rule took or no text consumed by match
                }
            }
        } else {
            // No more multi-line comment starts found in the remainder of the block.
            break;
        }
    }
    // If we've processed the entire line and no state is set (e.g., all comments ended, or no comments started),
    // ensure the block state is 0. This should be handled by matchMultiLineCommentAndFormat for comments that end.
    // If the loop finishes because currentPosition >= text.length() and currentBlockState() is still 0, it's correct.
    // If a comment started and went to EOB, currentBlockState() will be non-zero and we would have returned earlier.
    // If a continued comment from previous block ended, currentBlockState() is 0.
    // This final check might be redundant if logic above is perfect, but can act as a safeguard.
    if (currentPosition >= text.length() && currentBlockState() != 0) {
        // This case should ideally be covered by returns within the loops if a stateful rule consumes to EOB.
        // If it's reached, it implies a state was set but the block processing didn't terminate via an EOB return.
        // This is unlikely given the current structure but if it happens, the state is already set.
    } else if (currentBlockState() != 0) {
        // A multi-line comment did not consume the entire block, but its state is set.
        // This is normal if the comment ends mid-block and then no other comments start.
        // However, if the loop exited (no more rules found) and state is non-zero,
        // it means that the last active comment did *not* end. This is fine, state should persist.
    } else {
        // Default: if no stateful comment is active at the end of the block, state must be 0.
         setCurrentBlockState(0);
    }
}
