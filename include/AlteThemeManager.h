#ifndef ALTETHEMEMANAGER_H
#define ALTETHEMEMANAGER_H

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QFont>
#include <QPalette>

class AlteThemeManager {
public:
    AlteThemeManager();

    bool loadTheme(const QString& filePath);
    void applyTheme(QApplication* app) const;

    QColor getColor(const QString& name, const QColor& defaultValue = Qt::black) const;
    QColor getSyntaxColor(const QString& name, const QColor& defaultValue = Qt::black) const;
    QString getStyleSheet(const QString& widgetName) const;


private:
    QJsonObject themeData;
    QJsonObject colors;
    QJsonObject syntaxColors;
    QJsonObject styles;
    QJsonObject syntaxHighlightingRules;
    QJsonObject fontInfo;

    QString generateGlobalStyleSheet() const;

public:
    QFont getApplicationFont(const QFont& defaultFont = QApplication::font()) const;
    QFont getEditorFont(const QFont& defaultFont = QApplication::font()) const;

public:
    QJsonObject getSyntaxRulesForLanguage(const QString& languageName) const;

    int getStylesObjectSizeForDebug() const;
};

#endif // ALTETHEMEMANAGER_H
