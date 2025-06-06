#include "AlteThemeManager.h"
#include <QJsonParseError>
#include <QTextStream> // Not directly used, but often useful with QFile
#include <QStyleFactory> // For potentially setting a base style like "Fusion"
#include <QFontDatabase>

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
    qDebug() << "Application font set to:" << appFont.family() << "Size:" << appFont.pointSize();

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
    qDebug() << "Global palette applied. Window background:" << globalPalette.color(QPalette::Window).name();

    // Generate and apply the global stylesheet
    // QString globalStyleSheet = generateGlobalStyleSheet(); // Commented out for individual application
    // if (!globalStyleSheet.isEmpty()) {
    //     qDebug() << "Global stylesheet generated. Length:" << globalStyleSheet.length();
    //     if (globalStyleSheet.length() < 3000) {
    //         qDebug().noquote() << "Full Global Stylesheet Content:\n" << globalStyleSheet;
    //     } else {
    //         qDebug().noquote() << "Global Stylesheet Content (first 1500 chars):\n" << globalStyleSheet.left(1500);
    //     }
    //     app->setStyleSheet(globalStyleSheet);
    // } else {
    //     qWarning() << "Generated global stylesheet is empty. Check theme JSON 'styles' section.";
    // }

    if (this->styles.isEmpty()) {
        qWarning() << "No styles found in theme JSON's 'styles' section to apply individually.";
    } else {
        qDebug() << "Attempting to apply styles individually...";
        for (auto styleIt = this->styles.constBegin(); styleIt != this->styles.constEnd(); ++styleIt) {
            QString widgetName = styleIt.key();
            QString originalStyleValue = styleIt.value().toString();
            QString processedStyleValue = originalStyleValue;

            for (auto colorIt = this->colors.constBegin(); colorIt != this->colors.constEnd(); ++colorIt) {
                QString placeholder = QString("%%%%%1%%%%").arg(colorIt.key());
                processedStyleValue.replace(placeholder, colorIt.value().toString());
            }

            QString individualRule = QString("%1 { %2 }").arg(widgetName, processedStyleValue);
            qDebug().noquote() << "Attempting to apply individual QSS rule:" << individualRule;

            QString currentStyleSheet = app->styleSheet();
            // QString currentStyleSheet = app->styleSheet(); // Start fresh each time
            // QString newStyleSheet = currentStyleSheet;
            // if (!currentStyleSheet.isEmpty()) {
            //     newStyleSheet += "\n"; // Add a newline separator if current sheet is not empty
            // }
            // newStyleSheet += individualRule;

            app->setStyleSheet(individualRule); // Apply only the current rule, replacing previous
            // It's hard to check for immediate warnings here without more complex Qt signal handling.
            // We will rely on the global Qt message handler for "Could not parse..."
            // If a rule is bad, the warning should appear after its application attempt.
        }
        qDebug() << "Clearing stylesheet after individual attempts.";
        app->setStyleSheet(""); // Clear the stylesheet
        // qDebug().noquote() << "Final stylesheet after applying individual rules:\n" << app->styleSheet(); // This will just be the last rule
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
    QStringList globalStylesList; // Renamed to avoid confusion with the 'styles' member
    if (styles.isEmpty()) {
        qWarning() << "No styles found in theme JSON's 'styles' section to generate global stylesheet.";
    }
    for (const QString& widgetName : styles.keys()) {
        QString styleValue = styles.value(widgetName).toString();
        for (auto it = colors.constBegin(); it != colors.constEnd(); ++it) {
            QString placeholder = QString("%%%%%1%%%%").arg(it.key()); // e.g., %%cyberPulse%%
            styleValue.replace(placeholder, it.value().toString());
        }
        QString styleEntry = QString("%1 { %2 }").arg(widgetName, styleValue);
        globalStylesList.append(styleEntry);
    }
    QString finalStyleSheet = globalStylesList.join("\n");
    qDebug() << "Generated global stylesheet with color replacement. Length:" << finalStyleSheet.length();
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
