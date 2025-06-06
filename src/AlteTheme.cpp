#include "AlteTheme.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication> // For path later if needed
#include <QDir>

AlteTheme::AlteTheme() {
    // Initialize with very basic fallback defaults
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(Qt::black); // Fallback text color
    syntaxFormats.insert("default", defaultFormat);
    editorColors.insert("text", Qt::black);
    editorColors.insert("background", Qt::white); // Fallback background
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
    themeType = rootObj["type"].toString("dark"); // default to dark

    // Parse editor colors (application palette colors)
    QJsonObject globalColorsObj = rootObj["colors"].toObject();
    for (auto it = globalColorsObj.begin(); it != globalColorsObj.end(); ++it) {
        // These are general editor colors, not syntax-specific ones unless resolved later
        editorColors.insert(it.key(), QColor(it.value().toString()));
    }

    // Parse syntax formats from the new "token_styles" section
    // This section maps style_keys directly to their definitions.
    QJsonObject tokenStylesObj = rootObj["token_styles"].toObject();
    if (tokenStylesObj.isEmpty()) {
        qWarning() << "Theme JSON" << filePath << "is missing 'token_styles' object. Syntax highlighting may be unstyled.";
    }

    for (auto it = tokenStylesObj.begin(); it != tokenStylesObj.end(); ++it) {
        QString style_key = it.key(); // e.g., "keyword", "comment", "string"
        QJsonObject formatDetails = it.value().toObject();
        QTextCharFormat charFormat;

        if (formatDetails.contains("color")) {
            QString colorValue = formatDetails["color"].toString();
            // Resolve color if it's a name from the theme's main "colors" palette, otherwise treat as hex/QColor string
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
        // Could add underline, etc. here
        syntaxFormats.insert(style_key, charFormat);
    }
    qDebug() << "Theme loaded:" << name << ". Parsed" << editorColors.count() << "global colors and" << syntaxFormats.count() << "token styles.";
    return true;
}

QTextCharFormat AlteTheme::getFormat(const QString &type) const {
    return syntaxFormats.value(type, syntaxFormats.value("default")); // Fallback to "default" format
}

QColor AlteTheme::getColor(const QString &type) const {
    return editorColors.value(type, type == "background" ? Qt::white : Qt::black); // Fallback for core colors
}
