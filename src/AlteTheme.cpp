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

    // Parse editor colors
    QJsonObject colorsObj = rootObj["colors"].toObject();
    for (auto it = colorsObj.begin(); it != colorsObj.end(); ++it) {
        editorColors.insert(it.key(), QColor(it.value().toString()));
    }

    // Parse syntax formats
    QJsonObject syntaxFormatsObj = rootObj["syntax_formats"].toObject();
    for (auto it = syntaxFormatsObj.begin(); it != syntaxFormatsObj.end(); ++it) {
        QJsonObject formatDetails = it.value().toObject();
        QTextCharFormat charFormat;
        if (formatDetails.contains("color")) {
            charFormat.setForeground(QColor(formatDetails["color"].toString()));
        }
        if (formatDetails.contains("background_color")) { // Optional background for tokens
            charFormat.setBackground(QColor(formatDetails["background_color"].toString()));
        }
        if (formatDetails.contains("font_weight") && formatDetails["font_weight"].toString() == "bold") {
            charFormat.setFontWeight(QFont::Bold);
        }
        if (formatDetails.contains("font_style") && formatDetails["font_style"].toString() == "italic") {
            charFormat.setFontItalic(true);
        }
        // Could add underline, etc.
        syntaxFormats.insert(it.key(), charFormat);
    }
    qDebug() << "Theme loaded:" << name << "with" << syntaxFormats.count() << "syntax formats.";
    return true;
}

QTextCharFormat AlteTheme::getFormat(const QString &type) const {
    return syntaxFormats.value(type, syntaxFormats.value("default")); // Fallback to "default" format
}

QColor AlteTheme::getColor(const QString &type) const {
    return editorColors.value(type, type == "background" ? Qt::white : Qt::black); // Fallback for core colors
}
