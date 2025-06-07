#ifndef ALTETHEME_H
#define ALTETHEME_H

#include <QString>
#include <QMap>
#include <QTextCharFormat>
#include <QColor>

class AlteTheme {
public:
    AlteTheme();
    bool loadThemeFromFile(const QString &filePath);
    QTextCharFormat getFormat(const QString &type) const;
    QColor getColor(const QString &type) const;

private:
    QString name;
    QString themeType;
    QMap<QString, QColor> editorColors;
    QMap<QString, QTextCharFormat> syntaxFormats;
};

#endif // ALTETHEME_H
