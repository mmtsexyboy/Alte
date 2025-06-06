#include "splashscreen.h"
#include <QApplication> // For QScreen, if needed, though not directly here now
#include <QScreen>      // For screen geometry
#include <QAnimationGroup>
#include <QFontMetrics>

#include <QParallelAnimationGroup> // Ensure this is included

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent),
      m_centralColumnHeight(0.0),
      m_centralColumnOpacity(0.0), // Start fully transparent for the glyph
      m_columnHeightAnimation(nullptr),
      m_columnOpacityAnimation(nullptr),
      m_centralColumnAnimationGroup(nullptr),
      m_glyphAnimationStarted(false) // Initialize new member
{
    setFixedSize(400, 300);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
    // WA_TranslucentBackground is important if the splash screen isn't always a full rectangle,
    // or for smooth edges if the glyph is drawn near edges.
    // For a fully black initial screen, this is fine.
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    // Setup for the animation
    m_centralColumnAnimationGroup = new QParallelAnimationGroup(this);

    m_columnHeightAnimation = new QPropertyAnimation(this, "centralColumnHeight", this);
    m_columnHeightAnimation->setEasingCurve(QEasingCurve::OutCubic); // Smooth growth for the line

    m_columnOpacityAnimation = new QPropertyAnimation(this, "centralColumnOpacity", this);
    m_columnOpacityAnimation->setEasingCurve(QEasingCurve::Linear); // Simple fade-in for the line

    // Animations will be added to group in startGlyphAnimation if needed, or configured here.
    m_centralColumnAnimationGroup->addAnimation(m_columnHeightAnimation);
    m_centralColumnAnimationGroup->addAnimation(m_columnOpacityAnimation);

    connect(m_centralColumnAnimationGroup, &QParallelAnimationGroup::finished, this, &SplashScreen::animationFinished);
}

SplashScreen::~SplashScreen()
{
    // Qt's parent-child system handles deletion of animations when m_centralColumnAnimationGroup is deleted,
    // and m_centralColumnAnimationGroup is deleted when 'this' (SplashScreen) is deleted.
}

void SplashScreen::startGlyphAnimation(int durationMs) {
    m_glyphAnimationStarted = true; // Signal that the glyph animation phase has begun

    // Height animation for the line (e.g., grows over 70% of duration)
    m_columnHeightAnimation->setDuration(durationMs); // Grow to full height over the main duration
    m_columnHeightAnimation->setStartValue(0.0);
    m_columnHeightAnimation->setEndValue(height() / 2.0); // Grow to half screen height (center)

    // Opacity animation for the line (e.g., fades in over 50% of duration, could be delayed)
    // For a simple parallel fade-in while growing:
    m_columnOpacityAnimation->setDuration(durationMs * 0.7); // Fade in slightly faster than full growth
    m_columnOpacityAnimation->setStartValue(0.0);      // Start fully transparent
    m_columnOpacityAnimation->setEndValue(1.0);        // End fully opaque

    m_centralColumnAnimationGroup->start();
    update(); // Ensure a repaint is scheduled
}

void SplashScreen::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_glyphAnimationStarted) {
        painter.fillRect(rect(), QColor(0, 0, 0)); // Fully opaque black for initial phase
    } else {
        // Background for glyph animation phase (still black)
        painter.fillRect(rect(), QColor(0, 0, 0));

        if (m_centralColumnHeight > 0) { // Only draw if line has some height
            qreal lineWidth = 4.0; // Define the thickness of the vertical line

            // Calculate X position for a centered vertical line
            qreal lineX = (width() - lineWidth) / 2.0;

            // Calculate Y position for the top of the line.
            // The line grows from the bottom (height()) upwards.
            // Its animated height is m_centralColumnHeight.
            // So, its top Y coordinate is height() - m_centralColumnHeight.
            qreal lineTopY = height() - m_centralColumnHeight;

            QRectF lineRect(lineX, lineTopY, lineWidth, m_centralColumnHeight);

            QColor cyberPulse(0, 199, 164); // #00c7a4

            // Apply the animated opacity to the painter for the line
            painter.setOpacity(m_centralColumnOpacity);
            painter.setBrush(cyberPulse); // Use solid color for the line
            painter.setPen(Qt::NoPen);    // No border for the line itself

            painter.drawRect(lineRect);
        }
    }
}
