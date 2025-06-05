#include "syntaxhighlighter.h"
#include <QDebug> // For debugging output

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Keyword format (e.g., int, void, class, if, else, for, while, return)
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bint\\b" << "\\bvoid\\b" << "\\bclass\\b" << "\\bif\\b" << "\\belse\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\breturn\\b" << "\\bdouble\\b" << "\\bfloat\\b"
                    << "\\bchar\\b" << "\\bconst\\b" << "\\bstruct\\b" << "\\benum\\b" << "\\bnamespace\\b"
                    << "\\bpublic\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bstatic\\b" << "\\bvirtual\\b"
                    << "\\bexplicit\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\bnullptr\\b" << "\\bdelete\\b"
                    << "\\bnew\\b" << "\\bthis\\b" << "\\btemplate\\b" << "\\btypename\\b" << "\\btry\\b"
                    << "\\bcatch\\b" << "\\bthrow\\b" << "\\busing\\b" << "\\bvolatile\\b"
                    << "\\boperator\\b" << "\\binline\\b" << "\\btypedef\\b" << "\\bunsigned\\b"
                    << "\\bsigned\\b" << "\\bshort\\b" << "\\blong\\b" << "\\bextern\\b" << "\\bswitch\\b"
                    << "\\bcase\\b" << "\\bdefault\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bgoto\\b"
                    << "\\bconst_cast\\b" << "\\bdynamic_cast\\b" << "\\breinterpret_cast\\b" << "\\bstatic_cast\\b"
                    << "\\bmutable\\b" << "\\bnoexcept\\b" << "\\bconstexpr\\b" << "\\bdecltype\\b" << "\\bauto\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Class Format (simple: word after 'class ')
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta); // A distinct color for class names
    // This rule is specifically handled in highlightBlock to apply to capture group 1
    rule.pattern = QRegularExpression("\\bclass\\s+([A-Za-z_][A-Za-z0-9_]*)");
    rule.format = classFormat;
    highlightingRules.append(rule); // Add it to rules so its pattern is iterated

    // Preprocessor directives (#include, #define, #if, etc.)
    preprocessorFormat.setForeground(QColor(128, 0, 128)); // Purple
    preprocessorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*#[A-Za-z_]+.*"); // Include the rest of the line
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // Single line comment
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("//[^\n]*"); // Corrected newline issue
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Multi-line comment (handled separately but format stored)
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // String literals ("...")
    quotationFormat.setForeground(QColor(200,100,50)); // Orange-brown
    rule.pattern = QRegularExpression("\"([^\"\\\\]|\\\\.)*\""); // Handles escaped quotes
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Character literals ('...')
    // Using the same quotationFormat for char literals
    rule.pattern = QRegularExpression("'([^'\\\\]|\\\\.)*'"); // Handles escaped chars
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Numbers (integers, hex, float, binary)
    numberFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegularExpression("\\b(?:0[xX][0-9a-fA-F]+|0[bB][01]+|[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)[ULf]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Basic function/method call highlighting (name followed by parenthesis)
    functionFormat.setForeground(Qt::blue);
    // functionFormat.setFontItalic(true); // Optional: make function names italic
    rule.pattern = QRegularExpression("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            // Special handling for class names to only color the name (capture group 1)
            if (rule.pattern.pattern() == "\\bclass\\s+([A-Za-z_][A-Za-z0-9_]*)") {
                 if (match.hasMatch() && match.lastCapturedIndex() >= 1) {
                    setFormat(match.capturedStart(1), match.capturedLength(1), rule.format); // Use rule.format which is classFormat
                 }
            } else {
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
    setCurrentBlockState(0);

    // Handle multi-line comments
    int startIndex = 0;
    if (previousBlockState() != 1) { // If previous block was not already in a comment
        QRegularExpressionMatch match = commentStartExpression.match(text);
        startIndex = match.capturedStart();
    } else { // Previous block was in a comment, so this block starts in a comment
        startIndex = 0; // Start highlighting from the beginning of the block
    }


    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength;
        if (endIndex == -1) { // Comment doesn't end in this block
            setCurrentBlockState(1); // Mark this block as being inside a multi-line comment
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
            // setCurrentBlockState(0); // Comment ends in this block - already default
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        // Move startIndex to after the current comment to find the next one
        // Avoid re-matching the same comment part if commentLength is 0 (should not happen with current logic)
        int nextSearchPos = startIndex + (commentLength > 0 ? commentLength : 1);
        if (nextSearchPos >= text.length()) break;

        QRegularExpressionMatch nextMatch = commentStartExpression.match(text, nextSearchPos);
        startIndex = nextMatch.capturedStart();
    }
}
