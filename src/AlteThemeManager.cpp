#include "AlteThemeManager.h"
#include <QJsonParseError>
#include <QTextStream> // Not directly used, but often useful with QFile
#include <QStyleFactory> // For potentially setting a base style like "Fusion"
#include <QFontDatabase>
#include <cstdio> // For fprintf

AlteThemeManager::AlteThemeManager() {
    // Initialize with a fallback or default theme if desired, or leave empty
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

    syntaxColors = themeData.value("syntax").toObject(); // Legacy
    qDebug() << "Loaded syntaxColors keys:" << syntaxColors.keys();

    styles = themeData.value("styles").toObject();
    qDebug() << "Loaded styles keys:" << styles.keys();

    syntaxHighlightingRules = themeData.value("syntax_highlighting").toObject();
    // Assuming syntaxHighlightingRules is an object of objects. If it's an array, adjust accordingly.
    qDebug() << "Loaded syntaxHighlightingRules keys:" << syntaxHighlightingRules.keys();

    fontInfo = themeData.value("font").toObject(); // Load font definitions
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
    // Test: print a color to see if 'this->colors' is accessible before getApplicationFont
    fprintf(stderr, "[applyTheme] Test color 'text' before getApplicationFont: %s (fprintf).\n", getColor("text", Qt::magenta).name().toUtf8().constData());
    fflush(stderr);

    QFont appFont = getApplicationFont(app->font()); // Pass current app font as default
    fprintf(stderr, "[applyTheme] Called getApplicationFont. Setting app font (fprintf).\n");
    fflush(stderr);
    app->setFont(appFont);
    qDebug() << "Application font set to:" << appFont.family() << "Size:" << appFont.pointSize();

    fprintf(stderr, "[applyTheme] About to set global palette (fprintf).\n");
    fflush(stderr);
    // Test: print a color to see if 'this->colors' is accessible before palette setup
    fprintf(stderr, "[applyTheme] Test color 'windowBackground' before palette: %s (fprintf).\n", getColor("windowBackground", Qt::magenta).name().toUtf8().constData());
    fflush(stderr);

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

    // if (this->styles.isEmpty()) {
    //     qWarning() << "No styles found in theme JSON's 'styles' section to apply individually.";
    // } else {
    //     qDebug() << "Attempting to apply styles individually...";
    //     for (auto styleIt = this->styles.constBegin(); styleIt != this->styles.constEnd(); ++styleIt) {
    //         QString widgetName = styleIt.key();
    //         QString originalStyleValue = styleIt.value().toString();
    //         QString processedStyleValue = originalStyleValue;

    //         for (auto colorIt = this->colors.constBegin(); colorIt != this->colors.constEnd(); ++colorIt) {
    //             QString placeholder = QString("%%%%%1%%%%").arg(colorIt.key());
    //             processedStyleValue.replace(placeholder, colorIt.value().toString());
    //         }

    //         QString individualRule = QString("%1 { %2 }").arg(widgetName, processedStyleValue);
    //         qDebug().noquote() << "Attempting to apply individual QSS rule:" << individualRule;

    //         QString currentStyleSheet = app->styleSheet();
    //         // QString currentStyleSheet = app->styleSheet(); // Start fresh each time
    //         // QString newStyleSheet = currentStyleSheet;
    //         // if (!currentStyleSheet.isEmpty()) {
    //         //     newStyleSheet += "\n"; // Add a newline separator if current sheet is not empty
    //         // }
    //         // newStyleSheet += individualRule;

    //         app->setStyleSheet(individualRule); // Apply only the current rule, replacing previous
    //         // It's hard to check for immediate warnings here without more complex Qt signal handling.
    //         // We will rely on the global Qt message handler for "Could not parse..."
    //         // If a rule is bad, the warning should appear after its application attempt.
    //     }
    //     qDebug() << "Clearing stylesheet after individual attempts.";
    //     app->setStyleSheet(""); // Clear the stylesheet
    //     // qDebug().noquote() << "Final stylesheet after applying individual rules:\n" << app->styleSheet(); // This will just be the last rule
    // }
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
    fprintf(stderr, "[generateGlobalStyleSheet] Entered function (fprintf).\n");
    fflush(stderr); // Ensure it's flushed

    qDebug() << "[generateGlobalStyleSheet] Entered function (qDebug).";
    qDebug() << "[generateGlobalStyleSheet] Number of entries in 'styles' map:" << styles.size();

    QStringList globalStylesList; // Renamed to avoid confusion with the 'styles' member
    if (styles.isEmpty()) {
        fprintf(stderr, "[generateGlobalStyleSheet] 'styles' map is empty (fprintf).\n");
        fflush(stderr);
        qWarning() << "No styles found in theme JSON's 'styles' section to generate global stylesheet.";
    }
    fprintf(stderr, "[generateGlobalStyleSheet] Starting generation of global stylesheet entries loop (fprintf).\n");
    fflush(stderr);
    qDebug() << "Starting generation of global stylesheet entries loop (qDebug)...";

    for (const QString& widgetName : styles.keys()) {
        fprintf(stderr, "[generateGlobalStyleSheet] Processing widgetName: %s (fprintf).\n", widgetName.toUtf8().constData());
        fflush(stderr);
        qDebug().noquote() << "Processing widgetName:" << widgetName;

        QString originalStyleValue = styles.value(widgetName).toString();
        fprintf(stderr, "[generateGlobalStyleSheet]   Original styleValue: %s (fprintf).\n", originalStyleValue.toUtf8().constData());
        fflush(stderr);
        qDebug().noquote() << "  Original styleValue:" << originalStyleValue;

        QString processedStyleValue = originalStyleValue;
        for (auto it = colors.constBegin(); it != colors.constEnd(); ++it) {
            QString placeholderToReplace = QString("%%%1%%").arg(it.key());
            QString colorValue = it.value().toString();

            fprintf(stderr, "[generateGlobalStyleSheet]     Looping for color key: '%s', placeholder: '%s', value: '%s' (fprintf).\n",
                    it.key().toUtf8().constData(),
                    placeholderToReplace.toUtf8().constData(),
                    colorValue.toUtf8().constData());
            fflush(stderr);
            qDebug().noquote() << "    Looping for color key:" << it.key()
                               << ", placeholder:" << placeholderToReplace
                               << ", value:" << colorValue;

            fprintf(stderr, "[generateGlobalStyleSheet]     processedStyleValue BEFORE replace for '%s': '%s' (fprintf).\n",
                    placeholderToReplace.toUtf8().constData(),
                    processedStyleValue.toUtf8().constData());
            fflush(stderr);
            qDebug().noquote() << "    processedStyleValue BEFORE replace for" << placeholderToReplace << ":" << processedStyleValue;

            // Perform the replacement
            processedStyleValue.replace(placeholderToReplace, colorValue);

            fprintf(stderr, "[generateGlobalStyleSheet]     processedStyleValue AFTER replace for '%s': '%s' (fprintf).\n",
                    placeholderToReplace.toUtf8().constData(),
                    processedStyleValue.toUtf8().constData());
            fflush(stderr);
            qDebug().noquote() << "    processedStyleValue AFTER replace for" << placeholderToReplace << ":" << processedStyleValue;
        }
        fprintf(stderr, "[generateGlobalStyleSheet]   Processed styleValue: %s (fprintf).\n", processedStyleValue.toUtf8().constData());
        fflush(stderr);
        qDebug().noquote() << "  Processed styleValue:" << processedStyleValue;

        QString styleEntry = QString("%1 { %2 }").arg(widgetName, processedStyleValue);
        fprintf(stderr, "[generateGlobalStyleSheet]   Generated styleEntry: %s (fprintf).\n", styleEntry.toUtf8().constData());
        fflush(stderr);
        qDebug().noquote() << "  Generated styleEntry:" << styleEntry;
        globalStylesList.append(styleEntry);
    }

    fprintf(stderr, "[generateGlobalStyleSheet] Joining QStringList (fprintf).\n");
    fflush(stderr);
    QString finalStyleSheet = globalStylesList.join("\n");

    fprintf(stderr, "[generateGlobalStyleSheet] Generated global stylesheet with color replacement. Length: %lld (fprintf).\n", static_cast<long long>(finalStyleSheet.length()));
    fflush(stderr);
    qDebug() << "Generated global stylesheet with color replacement. Length:" << finalStyleSheet.length();

    if (finalStyleSheet.length() < 3000) { // Also print final stylesheet if short
        fprintf(stderr, "[generateGlobalStyleSheet] Final Global Stylesheet Content (fprintf):\n%s\n", finalStyleSheet.toUtf8().constData());
        fflush(stderr);
        qDebug().noquote() << "Final Global Stylesheet Content (qDebug):\n" << finalStyleSheet;
    }
    fprintf(stderr, "[generateGlobalStyleSheet] Returning final stylesheet (fprintf).\n");
    fflush(stderr);
    return finalStyleSheet;
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
    QString requestedFamily = fontInfo.value("applicationFontFamily").toString(defaultFont.family());
    int size = fontInfo.value("applicationFontSize").toInt(defaultFont.pointSize());
    if (size <= 0) size = defaultFont.pointSize(); // Ensure valid size

    QFontDatabase fontDatabase; // Create an instance
    QFont testFont(requestedFamily);
    if (testFont.family().compare(requestedFamily, Qt::CaseInsensitive) != 0 &&
        !fontDatabase.families().contains(requestedFamily, Qt::CaseInsensitive)) { // Call on instance
        qWarning() << "Application font" << requestedFamily << "not found. Attempting fallbacks.";
        QStringList fallbacks;
        fallbacks << "Monospace" << "DejaVu Sans Mono" << "Courier New" << "Courier"; // DejaVu added here as a common good one
        font.setFamilies(fallbacks);
    } else {
        font.setFamily(requestedFamily);
    }
    font.setPointSize(size);
    qDebug() << "Application font set to family:" << font.family() << "size:" << font.pointSize() << "(requested:" << requestedFamily << ")";
    return font;
}

QFont AlteThemeManager::getEditorFont(const QFont& defaultFont) const {
    QFont font(defaultFont); // Start with the passed default
    QString requestedFamily = fontInfo.value("editorFontFamily").toString(defaultFont.family());
    int size = fontInfo.value("editorFontSize").toInt(defaultFont.pointSize());
    if (size <= 0) size = defaultFont.pointSize(); // Ensure valid size

    QFontDatabase fontDatabase; // Create an instance
    QFont testFont(requestedFamily);
    if (testFont.family().compare(requestedFamily, Qt::CaseInsensitive) != 0 &&
        !fontDatabase.families().contains(requestedFamily, Qt::CaseInsensitive)) { // Call on instance
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

// Method for debugging
int AlteThemeManager::getStylesObjectSizeForDebug() const {
    return styles.size();
}
