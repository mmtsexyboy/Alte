#include "AlteThemeManager.h"
#include <QJsonParseError>
#include <QTextStream>
#include <QStyleFactory>
#include <QFontDatabase>
#include <cstdio>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFileInfoList> // Added for getAvailableThemes
#include <QCoreApplication> // Added for applicationDirPath in getAvailableThemes
// QDir, QFileInfo, QJsonDocument, QJsonObject, QJsonParseError are included via AlteThemeManager.h or already present

AlteThemeManager::AlteThemeManager() {
    // Consider a default path or a path from settings later
    // For now, let's assume syntax definitions are in a known relative path
    // This path should be relative to where the application expects resources.
    // For development, this might be relative to the build directory or source directory.
    // For deployment, it should be a path within the installed application bundle.
    // Let's use a path that might work if "resources" is alongside the executable.
    // Alternatively, it could be ":/syntax/" if using Qt Resource System.
    loadLanguageDefinitions("resources/syntax/");
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
    if (m_languageDefinitions.contains(languageName)) {
        // The stored QJsonObject IS the language definition itself,
        // which contains "highlighting_rules", "file_extensions", etc.
        // AlteSyntaxHighlighter expects the object that *contains* "highlighting_rules" array.
        return m_languageDefinitions.value(languageName);
    }
    qWarning() << "Syntax rules (language definition) not found for language:" << languageName << "in m_languageDefinitions.";
    // Attempt to fallback to theme's embedded rules if still desired (though goal is to remove them)
    // For this refactoring step, we strictly use m_languageDefinitions.
    // If a language definition (e.g. "cpp.json") was not loaded into m_languageDefinitions,
    // or if languageName is something like "C++" but the file was "cpp.json" and loaded as "cpp",
    // then it won't be found. This is expected.
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

void AlteThemeManager::loadLanguageDefinitions(const QString& directoryPath) {
    m_languageDefinitions.clear();
    QDir syntaxDir(directoryPath);
    if (!syntaxDir.exists()) {
        qWarning() << "Syntax definition directory does not exist:" << directoryPath;
        // Try a path relative to the application executable for deployed scenarios
        QDir appDir(QApplication::applicationDirPath());
        QString fallbackPath = appDir.filePath(directoryPath);
        qDebug() << "Attempting fallback syntax definition directory:" << fallbackPath;
        syntaxDir.setPath(fallbackPath);
        if (!syntaxDir.exists()) {
            qWarning() << "Fallback syntax definition directory also does not exist:" << fallbackPath;
            // If using Qt resources, the path might be like ":/syntax/"
            // Check one more common location for development: relative to current working dir
            QDir currentDir(QDir::currentPath());
            QString devPath = currentDir.filePath(directoryPath);
             qDebug() << "Attempting development syntax definition directory:" << devPath;
            syntaxDir.setPath(devPath);
             if (!syntaxDir.exists()) {
                qWarning() << "Development syntax definition directory also not found:" << devPath;
                 // Try Qt resource system path
                syntaxDir.setPath(":/syntax");
                if(!syntaxDir.exists()){
                    qWarning() << "Qt resource path ':/syntax' also not found. No language definitions will be loaded.";
                    return;
                } else {
                     qDebug() << "Found syntax definitions in Qt resource path ':/syntax'";
                }
            } else {
                 qDebug() << "Found syntax definitions in development path:" << devPath;
            }
        } else {
            qDebug() << "Found syntax definitions in fallback path:" << fallbackPath;
        }
    } else {
         qDebug() << "Found syntax definitions in primary path:" << directoryPath;
    }


    QStringList filters;
    filters << "*.json";
    QFileInfoList fileList = syntaxDir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QFile langFile(fileInfo.filePath());
        if (!langFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open language file:" << fileInfo.filePath();
            continue;
        }
        QByteArray langData = langFile.readAll();
        langFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(langData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing language JSON" << fileInfo.fileName() << ":" << parseError.errorString();
            continue;
        }
        if (!doc.isObject()) {
            qWarning() << "Language JSON is not an object:" << fileInfo.fileName();
            continue;
        }

        QJsonObject langObject = doc.object();
        QString langName = langObject.value("language_name").toString();
        if (langName.isEmpty()) {
            langName = fileInfo.baseName(); // Fallback to filename without extension
            qWarning() << "Language file" << fileInfo.fileName() << "is missing 'language_name'. Using filename '" << langName << "' as language name.";
        }

        // Ensure essential keys are present
        if (!langObject.contains("file_extensions") || !langObject.value("file_extensions").isArray()) {
            qWarning() << "Language" << langName << "is missing 'file_extensions' array. Skipping.";
            continue;
        }

        m_languageDefinitions.insert(langName, langObject);
        qDebug() << "Loaded language definition:" << langName << "from" << fileInfo.fileName();
    }
    if(m_languageDefinitions.isEmpty()){
        qWarning() << "No language definitions loaded. Syntax highlighting might not work as expected.";
    } else {
        qDebug() << "Total languages loaded:" << m_languageDefinitions.count();
    }
}

QString AlteThemeManager::detectLanguage(const QString& filePath, const QString& firstLineContent) const {
    if (m_languageDefinitions.isEmpty()) {
        qWarning() << "No language definitions loaded. Cannot detect language.";
        return QString(); // Or "Plain Text"
    }

    QFileInfo fileInfo(filePath);
    QString fileSuffix = fileInfo.suffix(); // e.g., "py"
    QString completeSuffix = fileInfo.completeSuffix(); // e.g., "tar.gz" - useful for multi-part extensions

    // Pass 1: Match by file extension
    for (auto it = m_languageDefinitions.constBegin(); it != m_languageDefinitions.constEnd(); ++it) {
        const QJsonObject& langDef = it.value();
        QJsonArray extensions = langDef.value("file_extensions").toArray();
        for (const QJsonValue& extVal : extensions) {
            QString ext = extVal.toString().mid(1); // Remove leading "." e.g. ".py" -> "py"
            if (!ext.isEmpty() && (ext == fileSuffix || ext == completeSuffix)) {
                qDebug() << "Detected language by extension:" << it.key() << "for file:" << filePath;
                return it.key();
            }
        }
    }

    // Pass 2: Match by first line content (if not empty)
    if (!firstLineContent.isEmpty()) {
        for (auto it = m_languageDefinitions.constBegin(); it != m_languageDefinitions.constEnd(); ++it) {
            const QJsonObject& langDef = it.value();
            if (langDef.contains("first_line_patterns")) {
                QJsonArray patterns = langDef.value("first_line_patterns").toArray();
                for (const QJsonValue& patternVal : patterns) {
                    QString patternStr = patternVal.toString();
                    if (patternStr.isEmpty()) continue;
                    QRegularExpression regex(patternStr);
                    if (regex.isValid() && regex.match(firstLineContent).hasMatch()) {
                        qDebug() << "Detected language by first line pattern:" << it.key() << "for file:" << filePath;
                        return it.key();
                    }
                }
            }
        }
    }

    qDebug() << "Language not detected for:" << filePath << ". Defaulting to Plain Text or empty.";
    // Fallback: if a "Plain Text" language is defined, use it for .txt or unknown
    if ( (fileSuffix == "txt" || completeSuffix == "txt") && m_languageDefinitions.contains("Plain Text")) {
        return "Plain Text";
    }
    // Return specific default if defined, otherwise empty
    return m_languageDefinitions.contains("Plain Text") ? "Plain Text" : QString();
}

QStringList AlteThemeManager::getAvailableLanguages() const {
    return m_languageDefinitions.keys();
}

QStringList AlteThemeManager::getExtensionsForLanguage(const QString& languageName) const {
    QStringList extensionsList;
    if (m_languageDefinitions.contains(languageName)) {
        const QJsonObject& langDef = m_languageDefinitions.value(languageName);
        QJsonArray extensions = langDef.value("file_extensions").toArray();
        for (const QJsonValue& extVal : extensions) {
            extensionsList.append(extVal.toString());
        }
    }
    return extensionsList;
}

QMap<QString, QString> AlteThemeManager::getAvailableThemes(const QString& directoryPath) const {
    QMap<QString, QString> availableThemes;
    QDir themesDir;

    // Attempt to find the themes directory using a similar fallback strategy as loadLanguageDefinitions
    QString effectivePath = directoryPath;
    if (!QFileInfo::exists(effectivePath)) {
        qDebug() << "Primary theme directory not found:" << effectivePath;
        effectivePath = QCoreApplication::applicationDirPath() + "/" + directoryPath;
        if (!QFileInfo::exists(effectivePath)) {
            qDebug() << "Theme directory relative to app path not found:" << effectivePath;
            effectivePath = QDir::currentPath() + "/" + directoryPath;
            if (!QFileInfo::exists(effectivePath)) {
                qDebug() << "Theme directory relative to current working dir not found:" << effectivePath;
                // Qt resource path for themes (e.g. ":/themes")
                // This assumes themes are also added to qrc if this path is to be used
                effectivePath = ":/themes";
                if (!QFileInfo::exists(effectivePath)) {
                   qWarning() << "Theme directory not found in standard locations including Qt resources ':/themes'. Cannot load available themes.";
                   return availableThemes;
                } else {
                   qDebug() << "Found themes in Qt resource path ':/themes'";
                }
            } else {
                qDebug() << "Found themes in CWD path:" << effectivePath;
            }
        } else {
            qDebug() << "Found themes in app path:" << effectivePath;
        }
    } else {
        qDebug() << "Found themes in primary path:" << effectivePath;
    }
    themesDir.setPath(effectivePath);

    if (!themesDir.exists()) {
        qWarning() << "Themes directory does not exist after all checks:" << themesDir.path();
        return availableThemes;
    }

    QStringList filters;
    filters << "*.json";
    QFileInfoList fileList = themesDir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QFile themeFile(fileInfo.filePath());
        if (!themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Could not open theme file for reading name:" << fileInfo.filePath();
            continue;
        }
        QByteArray jsonData = themeFile.readAll();
        themeFile.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QString themeName = doc.object().value("name").toString();
            if (!themeName.isEmpty()) {
                availableThemes.insert(themeName, fileInfo.canonicalFilePath());
                qDebug() << "Discovered theme:" << themeName << "at" << fileInfo.canonicalFilePath();
            } else {
                qWarning() << "Theme file" << fileInfo.fileName() << "is missing 'name' property.";
            }
        } else {
            qWarning() << "Error parsing theme file" << fileInfo.fileName() << ":" << parseError.errorString();
        }
    }
    return availableThemes;
}
