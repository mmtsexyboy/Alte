#include <QApplication>
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
#include <QDir>       // Required for QDir::homePath() in main() for file dialogs, but MainWindow uses it too. Keep for main, MainWindow.cpp has it.
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

#include "include/MainWindow.h" // Include the new MainWindow header

// Forward declare AlteThemeManager if its definition isn't needed in this header part
// class AlteThemeManager; // Not needed here as AlteThemeManager.h is included by AlteThemeManager.h (indirectly if main needs it) or directly.
// <QTimer> // Moved
// <QEvent> // Moved


// MainWindow class definition and implementations are now in MainWindow.h and MainWindow.cpp


// #include "main.moc" // Should be handled by build system for MainWindow. Q_OBJECT is not in main.cpp anymore.

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application language and layout direction for Persian
    QLocale persianLocale(QLocale::Persian, QLocale::Iran);
    QLocale::setDefault(persianLocale);
    app.setLayoutDirection(Qt::RightToLeft);

    // ThemeManager setup
    AlteThemeManager themeManager;
    // Construct path to theme file relative to application executable
    // Assuming themes are in: <app_dir>/../resources/themes/ (common for deployed apps)
    // or <app_dir>/resources/themes/ (common for build dir execution)
    QString themeFilePath = QCoreApplication::applicationDirPath() + "/resources/themes/default_dark_neon.json";
    QFile themeFile(themeFilePath);
    if (!themeFile.exists()) { // Fallback for development environment structure
        themeFilePath = QCoreApplication::applicationDirPath() + "/../resources/themes/default_dark_neon.json";
        if (!QFile(themeFilePath).exists()) {
             themeFilePath = "./resources/themes/default_dark_neon.json"; // Even further fallback
        }
    }
    qDebug() << "Final theme file path attempted:" << themeFilePath;
    if (themeManager.loadTheme(themeFilePath)) {
        // qDebug() << "Theme loaded successfully from:" << themeFilePath; // Potentially problematic
        qDebug() << "MAIN: loadTheme call completed."; // Test with simple string literal

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
