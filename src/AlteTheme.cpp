#include "AlteTheme.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

AlteTheme::AlteTheme() {
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(Qt::black);
    syntaxFormats.insert("default", defaultFormat);
    editorColors.insert("text", Qt::black);
    editorColors.insert("background", Qt::white);
}

bool AlteTheme::loadThemeFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Theme file does not exist:" << filePath;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open theme file:" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

    if (doc.isNull()) {
        qWarning() << "Failed to parse theme JSON from" << filePath << ":" << error.errorString();
        return false;
    }

    QJsonObject rootObj = doc.object();
    name = rootObj["name"].toString("Untitled Theme");
    themeType = rootObj["type"].toString("dark");

    QJsonObject globalColorsObj = rootObj["colors"].toObject();
    for (auto it = globalColorsObj.begin(); it != globalColorsObj.end(); ++it) {
        editorColors.insert(it.key(), QColor(it.value().toString()));
    }

    QJsonObject tokenStylesObj = rootObj["token_styles"].toObject();
    if (tokenStylesObj.isEmpty()) {
        qWarning() << "Theme JSON" << filePath << "is missing 'token_styles' object. Syntax highlighting may be unstyled.";
    }

    for (auto it = tokenStylesObj.begin(); it != tokenStylesObj.end(); ++it) {
        QString style_key = it.key();
        QJsonObject formatDetails = it.value().toObject();
        QTextCharFormat charFormat;

        if (formatDetails.contains("color")) {
            QString colorValue = formatDetails["color"].toString();
            if (editorColors.contains(colorValue)) {
                charFormat.setForeground(editorColors.value(colorValue));
            } else {
                charFormat.setForeground(QColor(colorValue));
            }
        }
        if (formatDetails.contains("background_color")) {
            QString bgColorValue = formatDetails["background_color"].toString();
            if (editorColors.contains(bgColorValue)) {
                charFormat.setBackground(editorColors.value(bgColorValue));
            } else {
                charFormat.setBackground(QColor(bgColorValue));
            }
        }
        if (formatDetails.contains("font_weight") && formatDetails["font_weight"].toString() == "bold") {
            charFormat.setFontWeight(QFont::Bold);
        }
        if (formatDetails.contains("font_style") && formatDetails["font_style"].toString() == "italic") {
            charFormat.setFontItalic(true);
        }
        syntaxFormats.insert(style_key, charFormat);
    }
    qDebug() << "Theme loaded:" << name << ". Parsed" << editorColors.count() << "global colors and" << syntaxFormats.count() << "token styles.";
    return true;
}

QTextCharFormat AlteTheme::getFormat(const QString &type) const {
    return syntaxFormats.value(type, syntaxFormats.value("default"));
}

QColor AlteTheme::getColor(const QString &type) const {
    return editorColors.value(type, type == "background" ? Qt::white : Qt::black);
}
