#ifndef ALTETHEMEMANAGER_H
#define ALTETHEMEMANAGER_H

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug> // For logging errors
#include <QApplication> // For applyTheme argument

class AlteThemeManager {
public:
    AlteThemeManager();

    bool loadTheme(const QString& filePath);
    void applyTheme(QApplication* app) const;

    QColor getColor(const QString& name, const QColor& defaultValue = Qt::black) const;
    QColor getSyntaxColor(const QString& name, const QColor& defaultValue = Qt::black) const;
    QString getStyleSheet(const QString& widgetName) const;


private:
    QJsonObject themeData; // Holds all theme data (colors, styles)
    QJsonObject colors;    // Specific "colors" map for quick lookup
    QJsonObject syntaxColors; // Legacy direct syntax color map (can be phased out or used as fallback)
    QJsonObject styles;    // Specific "styles" map (stylesheets for widgets)
    QJsonObject syntaxHighlightingRules; // Holds all language-specific rules
    QJsonObject fontInfo; // Holds font definitions

    QString generateGlobalStyleSheet() const;

public: // Public accessors for fonts
    QFont getApplicationFont(const QFont& defaultFont = QApplication::font()) const;
    QFont getEditorFont(const QFont& defaultFont = QApplication::font()) const;

public: // Make it public for SyntaxHighlighter to access easily
    QJsonObject getSyntaxRulesForLanguage(const QString& languageName) const;
};

#endif // ALTETHEMEMANAGER_H
