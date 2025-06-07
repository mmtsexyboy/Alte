#include "AlteThemeManager.h"
#include <QJsonParseError>
#include <QTextStream>
#include <QStyleFactory>
#include <QFontDatabase>
#include <cstdio>

AlteThemeManager::AlteThemeManager() {
}

bool AlteThemeManager::loadTheme(const QString& filePath) {
    qDebug() << "Attempting to load theme from:" << filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open theme file:" << filePath << file.errorString();
        qDebug() << "QFile error:" << file.errorString();
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
        qDebug() << "Theme JSON content:" << jsonData;
        return false;
    }

    themeData = doc.object();
    qDebug() << "Theme JSON parsed successfully. Name:" << themeData.value("name").toString();

    colors = themeData.value("colors").toObject();
    qDebug() << "Loaded colors keys:" << colors.keys();

    syntaxColors = themeData.value("syntax").toObject();
    qDebug() << "Loaded syntaxColors keys:" << syntaxColors.keys();

    styles = themeData.value("styles").toObject();
    qDebug() << "Loaded styles keys:" << styles.keys();

    syntaxHighlightingRules = themeData.value("syntax_highlighting").toObject();
    qDebug() << "Loaded syntaxHighlightingRules keys:" << syntaxHighlightingRules.keys();

    fontInfo = themeData.value("font").toObject();
    qDebug() << "Loaded fontInfo keys:" << fontInfo.keys();

    qInfo() << "Theme loaded successfully:" << themeData.value("name").toString();
    return true;
}

void AlteThemeManager::applyTheme(QApplication* app) const {
    fprintf(stderr, "[applyTheme] Entered function (fprintf).\n");
    fflush(stderr);

    if (!app) {
        qWarning() << "QApplication instance is null. Cannot apply theme.";
        fprintf(stderr, "[applyTheme] QApplication instance is null. Cannot apply theme (fprintf).\n");
        fflush(stderr);
        return;
    }

    fprintf(stderr, "[applyTheme] Setting Fusion style (fprintf).\n");
    fflush(stderr);
    app->setStyle(QStyleFactory::create("Fusion"));

    fprintf(stderr, "[applyTheme] About to call getApplicationFont (fprintf).\n");
    fflush(stderr);
    fprintf(stderr, "[applyTheme] Test color 'text' before getApplicationFont: %s (fprintf).\n", getColor("text", Qt::magenta).name().toUtf8().constData());
    fflush(stderr);

    QFont appFont = getApplicationFont(app->font());
    fprintf(stderr, "[applyTheme] Called getApplicationFont. Setting app font (fprintf).\n");
    fflush(stderr);
    app->setFont(appFont);
    qDebug() << "Application font set to:" << appFont.family() << "Size:" << appFont.pointSize();

    fprintf(stderr, "[applyTheme] About to set global palette (fprintf).\n");
    fflush(stderr);
    fprintf(stderr, "[applyTheme] Test color 'windowBackground' before palette: %s (fprintf).\n", getColor("windowBackground", Qt::magenta).name().toUtf8().constData());
    fflush(stderr);

    QPalette globalPalette;
    globalPalette.setColor(QPalette::Window, getColor("windowBackground", Qt::white));
    globalPalette.setColor(QPalette::WindowText, getColor("text", Qt::black));
    globalPalette.setColor(QPalette::Base, getColor("base", Qt::white));
    globalPalette.setColor(QPalette::AlternateBase, getColor("alternateBase", Qt::lightGray));
    globalPalette.setColor(QPalette::ToolTipBase, getColor("tooltipBase", Qt::white));
    globalPalette.setColor(QPalette::ToolTipText, getColor("tooltipText", Qt::black));
    globalPalette.setColor(QPalette::Text, getColor("text", Qt::black));
    globalPalette.setColor(QPalette::Disabled, QPalette::Text, getColor("textDisabled", Qt::darkGray));
    globalPalette.setColor(QPalette::Button, getColor("button", Qt::lightGray));
    globalPalette.setColor(QPalette::ButtonText, getColor("buttonText", Qt::black));
    globalPalette.setColor(QPalette::Disabled, QPalette::ButtonText, getColor("textDisabled", Qt::darkGray));
    globalPalette.setColor(QPalette::BrightText, Qt::red);
    globalPalette.setColor(QPalette::Link, getColor("accent", Qt::blue));
    globalPalette.setColor(QPalette::Highlight, getColor("highlight", Qt::blue));
    globalPalette.setColor(QPalette::HighlightedText, getColor("highlightedText", Qt::white));

    fprintf(stderr, "[applyTheme] Palette colors assigned. Applying palette to app (fprintf).\n");
    fflush(stderr);
    app->setPalette(globalPalette);
    qDebug() << "Global palette applied. Window background:" << globalPalette.color(QPalette::Window).name();

    fprintf(stderr, "[applyTheme] About to call generateGlobalStyleSheet (fprintf).\n");
    fflush(stderr);
    // Test: print a color to see if 'this->colors' is accessible just before generateGlobalStyleSheet
    fprintf(stderr, "[applyTheme] Test color 'accent' before generateGlobalStyleSheet: %s (fprintf).\n", getColor("accent", Qt::magenta).name().toUtf8().constData());
    fflush(stderr);

    QString globalStyleSheet = generateGlobalStyleSheet();
    fprintf(stderr, "[applyTheme] Called generateGlobalStyleSheet. Result length: %lld (fprintf).\n", static_cast<long long>(globalStyleSheet.length()));
    fflush(stderr);

    if (!globalStyleSheet.isEmpty()) {
        qDebug() << "Global stylesheet generated. Length:" << globalStyleSheet.length();
        if (globalStyleSheet.length() < 3000) {
            qDebug().noquote() << "Full Global Stylesheet Content:\n" << globalStyleSheet;
        } else {
            qDebug().noquote() << "Global Stylesheet Content (first 1500 chars):\n" << globalStyleSheet.left(1500);
        }
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
    qDebug() << ">>> generateGlobalStyleSheet V3 executing <<<";

    QStringList globalStylesList;

    if (styles.isEmpty()) {
        qWarning() << "No styles found in theme JSON's 'styles' section to generate global stylesheet.";
    }

    for (const QString& widgetName : styles.keys()) {
        QString originalStyleValue = styles.value(widgetName).toString();
        QString processedStyleValue = originalStyleValue;
        for (auto it = colors.constBegin(); it != colors.constEnd(); ++it) {
            QString placeholderToReplace = QString("%%%1%%").arg(it.key());
            QString colorValue = it.value().toString();
            processedStyleValue.replace(placeholderToReplace, colorValue);
        }
        QString styleEntry = QString("%1 { %2 }").arg(widgetName, processedStyleValue);
        globalStylesList.append(styleEntry);
    }

    QString finalStyleSheet = globalStylesList.join("\n");
    return finalStyleSheet;
}

QJsonObject AlteThemeManager::getSyntaxRulesForLanguage(const QString& languageName) const {
    if (syntaxHighlightingRules.contains(languageName)) {
        return syntaxHighlightingRules.value(languageName).toObject();
    }
    qWarning() << "Syntax rules not found for language:" << languageName;
    return QJsonObject();
}

QFont AlteThemeManager::getApplicationFont(const QFont& defaultFont) const {
    QFont font(defaultFont);
    QString requestedFamily = fontInfo.value("applicationFontFamily").toString(defaultFont.family());
    int size = fontInfo.value("applicationFontSize").toInt(defaultFont.pointSize());
    if (size <= 0) size = defaultFont.pointSize();

    QFontDatabase fontDatabase;
    QFont testFont(requestedFamily);
    if (testFont.family().compare(requestedFamily, Qt::CaseInsensitive) != 0 &&
        !fontDatabase.families().contains(requestedFamily, Qt::CaseInsensitive)) {
        qWarning() << "Application font" << requestedFamily << "not found. Attempting fallbacks.";
        QStringList fallbacks;
        fallbacks << "Monospace" << "DejaVu Sans Mono" << "Courier New" << "Courier";
        font.setFamilies(fallbacks);
    } else {
        font.setFamily(requestedFamily);
    }
    font.setPointSize(size);
    qDebug() << "Application font set to family:" << font.family() << "size:" << font.pointSize() << "(requested:" << requestedFamily << ")";
    return font;
}

QFont AlteThemeManager::getEditorFont(const QFont& defaultFont) const {
    QFont font(defaultFont);
    QString requestedFamily = fontInfo.value("editorFontFamily").toString(defaultFont.family());
    int size = fontInfo.value("editorFontSize").toInt(defaultFont.pointSize());
    if (size <= 0) size = defaultFont.pointSize();

    QFontDatabase fontDatabase;
    QFont testFont(requestedFamily);
    if (testFont.family().compare(requestedFamily, Qt::CaseInsensitive) != 0 &&
        !fontDatabase.families().contains(requestedFamily, Qt::CaseInsensitive)) {
        qWarning() << "Editor font" << requestedFamily << "not found. Attempting fallbacks.";
        QStringList fallbacks;
        fallbacks << "Monospace" << "DejaVu Sans Mono" << "Courier New" << "Courier";
        font.setFamilies(fallbacks);
    } else {
        font.setFamily(requestedFamily);
    }
    font.setPointSize(size);
    qDebug() << "Editor font set to family:" << font.family() << "size:" << font.pointSize() << "(requested:" << requestedFamily << ")";
    return font;
}

int AlteThemeManager::getStylesObjectSizeForDebug() const {
    return styles.size();
}
