#include <QApplication>
#include <QSurfaceFormat> // Added for GPU rendering promotion
// <QMainWindow> // Moved to MainWindow.h
// <QTextEdit> // Moved to MainWindow.h/cpp
// <QMenuBar> // Implicitly used by menuBar(), handled by QMainWindow
// <QMenu> // Moved to MainWindow.cpp
// <QAction> // Moved to MainWindow.h/cpp
// <QFileDialog> // Moved to MainWindow.cpp
#include <QFile> // Still needed for theme loading in main()
// <QTextStream> // Moved to MainWindow.cpp
// <QMessageBox> // Moved to MainWindow.cpp
// <QString> // Moved to MainWindow.h
// <QFileInfo> // Moved to MainWindow.cpp
// <QCloseEvent> // Moved to MainWindow.h
#include <QLocale>    // Required for QLocale
#include <QDir>       // Required for QDir
#include <QStandardPaths> // Required for QStandardPaths (though not directly used in this iteration, good for future)
// "AlteSyntaxHighlighter.h" // Moved to MainWindow.h/cpp
#include "splashscreen.h"      // For SplashScreen
#include "AlteThemeManager.h"  // For ThemeManager
// <QTimer> // Moved to MainWindow.h/cpp (one QTimer for splash, one for MainWindow focus)
#include <QScreen>             // For QScreen to center splash
#include <QCoreApplication>    // For applicationDirPath
#include <QIcon>               // For QIcon (used in MainWindow constructor, now in MainWindow.cpp)
#include <stdexcept>           // For std::exception
#include <QDebug>              // For qDebug messages
#include <cstdio>              // For fprintf
#include <QFileInfo>           // For QFile::exists() and QFileInfo::exists()

#include "MainWindow.h" // Include the new MainWindow header

// Forward declare AlteThemeManager if its definition isn't needed in this header part
// class AlteThemeManager; // Not needed here as AlteThemeManager.h is included by AlteThemeManager.h (indirectly if main needs it) or directly.
// <QTimer> // Moved
// <QEvent> // Moved


// MainWindow class definition and implementations are now in MainWindow.h and MainWindow.cpp


// #include "main.moc" // Should be handled by build system for MainWindow. Q_OBJECT is not in main.cpp anymore.

int main(int argc, char *argv[]) {
    // Attempt to set a default surface format to promote hardware acceleration
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL); // Specify OpenGL
    fmt.setProfile(QSurfaceFormat::CoreProfile);   // Request Core Profile
    fmt.setVersion(3, 3); // Request OpenGL 3.3 (widely compatible)
    // You could also request stencil buffer, depth buffer, samples for MSAA, etc. if needed
    // fmt.setDepthBufferSize(24);
    // fmt.setStencilBufferSize(8);
    // fmt.setSamples(4); // For 4x MSAA
    QSurfaceFormat::setDefaultFormat(fmt);

    // Additionally, you can set attributes like AA_UseDesktopOpenGL
    // It's often good practice to set these before the QApplication is instantiated.
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // For high-DPI displays, if not already handled elsewhere:
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    // Log current working directory and application path
    qDebug() << "Current working directory (PWD):" << QDir::currentPath();
    QString appPath = QCoreApplication::applicationDirPath();
    qDebug() << "Application directory path (appPath):" << appPath;

    // Set application language and layout direction for Persian
    QLocale persianLocale(QLocale::Persian, QLocale::Iran);
    QLocale::setDefault(persianLocale);
    app.setLayoutDirection(Qt::RightToLeft);

    // ThemeManager setup
    AlteThemeManager themeManager;

    // QString appPath = QCoreApplication::applicationDirPath(); // Moved up
    // qDebug() << "Application directory path:" << appPath; // Moved up

    QString themeFilePath = "";
    QString themeName = "default_dark_neon.json"; // Define theme name

    QList<QString> potentialBasePaths;
    potentialBasePaths.append(appPath + "/../share/alte/resources"); // a. /opt/alte/bin/../share/alte/resources -> /opt/alte/share/alte/resources
    potentialBasePaths.append(appPath + "/resources");                // b. <app_dir>/resources
    potentialBasePaths.append(appPath + "/../resources");             // c. <app_dir>/../resources (e.g. build/bin/ -> build/resources)
    potentialBasePaths.append(appPath + "/../../resources");          // d. <app_dir>/../../resources (e.g. build/bin/ -> resources)

    for (const QString& basePath : potentialBasePaths) {
        QString currentThemePath = basePath + "/themes/" + themeName;
        qDebug() << "Attempting resource path:" << currentThemePath;
        QFileInfo fileInfo(currentThemePath);
        if (fileInfo.exists() && fileInfo.isFile()) {
            themeFilePath = currentThemePath;
            qDebug() << "Theme file found at:" << themeFilePath;
            break;
        } else {
            qDebug() << "Theme file not found at:" << currentThemePath;
        }
    }

    if (themeFilePath.isEmpty()) {
        qWarning() << "Could not find theme in standard locations. Trying directly relative to app executable.";
        QString directRelativePath = appPath + "/resources/themes/" + themeName;
        QFileInfo fileInfo(directRelativePath);
        if (fileInfo.exists() && fileInfo.isFile()) {
            themeFilePath = directRelativePath;
            qDebug() << "Theme file found at direct relative path:" << themeFilePath;
        } else {
            // If even that fails, as a last resort, use the original problematic relative path
            // but warn more strongly.
            QString finalFallbackPath = "./resources/themes/" + themeName;
            qWarning() << "Direct relative path failed (" << directRelativePath << "). Critical fallback to './resources/themes/'. This is very likely to fail unless CWD is project root:" << finalFallbackPath;
            themeFilePath = finalFallbackPath;
        }
    }

    // Ensure themeFilePath is normalized to avoid issues with ".." or "."
    if (!themeFilePath.isEmpty()) {
        QFileInfo fileInfo(themeFilePath);
        themeFilePath = fileInfo.absoluteFilePath(); // Normalize the path
        qDebug() << "Normalized theme file path to be used:" << themeFilePath;
    }


    if (themeManager.loadTheme(themeFilePath)) {
        qDebug() << "MAIN: loadTheme call completed for:" << themeFilePath;

        // Simplest possible fprintf test
        fprintf(stderr, "MAIN: fprintf test immediately after loadTheme returns.\n");
        fflush(stderr);

        // Restore themeManager integrity checks
        fprintf(stderr, "MAIN: Test after loadTheme, before applyTheme. Color 'text': %s\n",
                themeManager.getColor("text", Qt::magenta).name().toUtf8().constData());
        fflush(stderr);
        fprintf(stderr, "MAIN: Test getting 'styles' object size: %d\n",
                themeManager.getStylesObjectSizeForDebug());
        fflush(stderr);

        themeManager.applyTheme(&app);
    } else {
        qWarning() << "Failed to load theme from:" << themeFilePath << ". Using default Qt appearance.";
        // Potentially add more specific error info here if possible or if loadTheme provides it.
    }

    SplashScreen splash; // SplashScreen is now a QWidget, not QSplashScreen
    // It's important that MainWindow is created *after* the theme is applied,
    // so it picks up themed styles and palette during its construction.
    MainWindow mainWindow(&themeManager); // Pass themeManager to MainWindow

    // Show and center SplashScreen AFTER MainWindow is created but BEFORE it's shown.
    // This ensures splash is on top.
    splash.show();
    if (QScreen *screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->geometry();
        splash.move((screenGeometry.width() - splash.width()) / 2,
                    (screenGeometry.height() - splash.height()) / 2);
    }

    // Connect the splash screen's animationFinished signal to show main window
    QObject::connect(&splash, &SplashScreen::animationFinished, &app, [&]() {
    qDebug() << "Lambda connected to animationFinished: Started.";
    try {
        qDebug() << "Splash animation finished. Attempting to show main window...";

        if (QScreen *screen = QApplication::primaryScreen()) {
            QRect screenGeometry = screen->availableGeometry(); // Use availableGeometry for usable space

            // Calculate desired size (e.g., 70% of screen dimensions)
            int desiredWidth = static_cast<int>(screenGeometry.width() * 0.70);
            int desiredHeight = static_cast<int>(screenGeometry.height() * 0.70);

            // Calculate top-left position to center the window
            int x = screenGeometry.x() + (screenGeometry.width() - desiredWidth) / 2;
            int y = screenGeometry.y() + (screenGeometry.height() - desiredHeight) / 2;

            mainWindow.setGeometry(x, y, desiredWidth, desiredHeight);
            qDebug() << "Set main window geometry to:" << x << y << desiredWidth << desiredHeight;
        } else {
            // Fallback if primary screen is not available (should be rare)
            qDebug() << "Primary screen not found, using fallback geometry for main window.";
            mainWindow.setGeometry(100, 100, 1024, 768); // A slightly larger default
        }
        // The extra setGeometry and brace that were here have been removed.

        qDebug() << "Before mainWindow.show()";
        mainWindow.show();
        qDebug() << "After mainWindow.show()";

        qDebug() << "Before mainWindow.newFile()";
        mainWindow.newFile(); // Initialize with newFile as before
        qDebug() << "After mainWindow.newFile()";

        qDebug() << "newFile() called. Activating window..."; // This existing log is fine
        mainWindow.activateWindow(); // Ensure main window gets focus
        qDebug() << "Main window activated."; // This existing log is fine

        qDebug() << "Before splash.close()";
        splash.close(); // Close the splash screen widget
        qDebug() << "After splash.close()";
        qDebug() << "Splash screen closed. Main window setup complete."; // This existing log is fine
    } catch (const std::exception& e) {
        qCritical() << "Std::exception caught during main window setup: " << e.what();
    } catch (...) {
        qCritical() << "Unknown exception caught during main window setup.";
    }
    });

    // splash.startAnimation(1500); // Old animation call
    // splash.startCentralColumnAnimation(500); // Old direct call, remove this

    // New: Show splash, then after a delay, start the glyph animation
    // The glyph animation itself will take 'glyphAnimationDurationMs'
    int initialBlackScreenMs = 750;
    int glyphAnimationDurationMs = 900; // Duration for the glyph to draw and fade in

    QTimer::singleShot(initialBlackScreenMs, &splash, [=, &splash]() {
        splash.startGlyphAnimation(glyphAnimationDurationMs);
    });

    return app.exec();
}
