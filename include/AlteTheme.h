#ifndef ALTETHEME_H
#define ALTETHEME_H

#include <QString>
#include <QMap>
#include <QTextCharFormat>
#include <QColor> // Include QColor

class AlteTheme {
public:
    AlteTheme();
    bool loadThemeFromFile(const QString &filePath);
    QTextCharFormat getFormat(const QString &type) const; // e.g., "keyword", "comment"
    QColor getColor(const QString &type) const; // e.g., "text", "background"

private:
    QString name;
    QString themeType; // "dark" or "light"
    QMap<QString, QColor> editorColors;
    QMap<QString, QTextCharFormat> syntaxFormats;
};

#endif // ALTETHEME_H
