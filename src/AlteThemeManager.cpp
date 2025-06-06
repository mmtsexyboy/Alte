#include "AlteThemeManager.h"
#include <QJsonParseError>
#include <QTextStream> // Not directly used, but often useful with QFile
#include <QStyleFactory> // For potentially setting a base style like "Fusion"

AlteThemeManager::AlteThemeManager() {
    // Initialize with a fallback or default theme if desired, or leave empty
}

bool AlteThemeManager::loadTheme(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open theme file:" << filePath << file.errorString();
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Error parsing theme JSON:" << parseError.errorString() << "at offset" << parseError.offset;
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Theme JSON is not an object.";
        return false;
    }

    themeData = doc.object();
    colors = themeData.value("colors").toObject();
    syntaxColors = themeData.value("syntax").toObject(); // Legacy
    styles = themeData.value("styles").toObject();
    syntaxHighlightingRules = themeData.value("syntax_highlighting").toObject();
    fontInfo = themeData.value("font").toObject(); // Load font definitions

    qInfo() << "Theme loaded successfully:" << themeData.value("name").toString();
    return true;
}

void AlteThemeManager::applyTheme(QApplication* app) const {
    if (!app) {
        qWarning() << "QApplication instance is null. Cannot apply theme.";
        return;
    }

    // It's often good to start from a clean base style like Fusion for consistency
    // before applying extensive stylesheets, especially if not all controls are styled.
    app->setStyle(QStyleFactory::create("Fusion"));

    // Apply application font
    QFont appFont = getApplicationFont(app->font()); // Pass current app font as default
    app->setFont(appFont);

    // Apply a global palette (optional, as stylesheet can override)
    QPalette globalPalette;
    globalPalette.setColor(QPalette::Window, getColor("windowBackground", Qt::white));
    globalPalette.setColor(QPalette::WindowText, getColor("text", Qt::black));
    globalPalette.setColor(QPalette::Base, getColor("base", Qt::white)); // Background for text inputs
    globalPalette.setColor(QPalette::AlternateBase, getColor("alternateBase", Qt::lightGray));
    globalPalette.setColor(QPalette::ToolTipBase, getColor("tooltipBase", Qt::white));
    globalPalette.setColor(QPalette::ToolTipText, getColor("tooltipText", Qt::black));
    globalPalette.setColor(QPalette::Text, getColor("text", Qt::black));
    globalPalette.setColor(QPalette::Disabled, QPalette::Text, getColor("textDisabled", Qt::darkGray));
    globalPalette.setColor(QPalette::Button, getColor("button", Qt::lightGray));
    globalPalette.setColor(QPalette::ButtonText, getColor("buttonText", Qt::black));
    globalPalette.setColor(QPalette::Disabled, QPalette::ButtonText, getColor("textDisabled", Qt::darkGray));
    globalPalette.setColor(QPalette::BrightText, Qt::red); // Used for highlighted text if not overridden
    globalPalette.setColor(QPalette::Link, getColor("accent", Qt::blue));
    globalPalette.setColor(QPalette::Highlight, getColor("highlight", Qt::blue)); // Selection background
    globalPalette.setColor(QPalette::HighlightedText, getColor("highlightedText", Qt::white)); // Selection text

    app->setPalette(globalPalette);

    // Generate and apply the global stylesheet
    QString globalStyleSheet = generateGlobalStyleSheet();
    if (!globalStyleSheet.isEmpty()) {
        app->setStyleSheet(globalStyleSheet);
    } else {
        qWarning() << "Generated global stylesheet is empty. Check theme JSON 'styles' section.";
    }
}

QColor AlteThemeManager::getColor(const QString& name, const QColor& defaultValue) const {
    return QColor(colors.value(name).toString(defaultValue.name()));
}

QColor AlteThemeManager::getSyntaxColor(const QString& name, const QColor& defaultValue) const {
     return QColor(syntaxColors.value(name).toString(defaultValue.name()));
}

QString AlteThemeManager::getStyleSheet(const QString& widgetName) const {
    return styles.value(widgetName).toString("");
}

QString AlteThemeManager::generateGlobalStyleSheet() const {
    QStringList globalStyles;
    for (const QString& widgetName : styles.keys()) {
        QString style = QString("%1 { %2 }").arg(widgetName, styles.value(widgetName).toString());
        globalStyles.append(style);
    }
    return globalStyles.join("\n");
}

QJsonObject AlteThemeManager::getSyntaxRulesForLanguage(const QString& languageName) const {
    if (syntaxHighlightingRules.contains(languageName)) {
        return syntaxHighlightingRules.value(languageName).toObject();
    }
    qWarning() << "Syntax rules not found for language:" << languageName;
    return QJsonObject(); // Return empty object if language not found
}

QFont AlteThemeManager::getApplicationFont(const QFont& defaultFont) const {
    QFont font(defaultFont); // Start with the passed default
    QString family = fontInfo.value("applicationFontFamily").toString(font.family());
    int size = fontInfo.value("applicationFontSize").toInt(font.pointSize());

    font.setFamily(family);
    if (size > 0) font.setPointSize(size);
    // Could add weight, italic etc. if defined in JSON for applicationFont
    return font;
}

QFont AlteThemeManager::getEditorFont(const QFont& defaultFont) const {
    QFont font(defaultFont); // Start with the passed default
    QString family = fontInfo.value("editorFontFamily").toString(font.family());
    int size = fontInfo.value("editorFontSize").toInt(font.pointSize());

    font.setFamily(family);
    if (size > 0) font.setPointSize(size);
    // Could add weight, italic etc. if defined in JSON for editorFont
    return font;
}
