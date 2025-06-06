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
#include <QMap> // Added for getAvailableThemes

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
    QMap<QString, QJsonObject> m_languageDefinitions;

    QString generateGlobalStyleSheet() const;

public:
    void loadLanguageDefinitions(const QString& directoryPath);
    QString detectLanguage(const QString& filePath, const QString& firstLineContent) const;
    QStringList getAvailableLanguages() const;
    QStringList getExtensionsForLanguage(const QString& languageName) const;

public:
    QFont getApplicationFont(const QFont& defaultFont = QApplication::font()) const;
    QFont getEditorFont(const QFont& defaultFont = QApplication::font()) const;

public:
    QJsonObject getSyntaxRulesForLanguage(const QString& languageName) const;

    int getStylesObjectSizeForDebug() const;

    QMap<QString, QString> getAvailableThemes(const QString& directoryPath = "resources/themes/") const;
};

#endif // ALTETHEMEMANAGER_H
