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

        if (m_centralColumnHeight > 0 && m_centralColumnOpacity > 0) {
            painter.setOpacity(m_centralColumnOpacity);
            QColor neonColor(0, 199, 164); // #00c7a4
            painter.setPen(QPen(neonColor, 3)); // Pen thickness 3

            // Define 'A' geometry based on splash screen size and m_centralColumnHeight
            // All coordinates relative to center of the splash screen.
            float centerX = width() / 2.0f;
            float bottomY = height() * 0.75f; // Baseline for the 'A'
            float charHeight = height() * 0.4f; // Max height of the 'A'
            float charWidth = charHeight * 0.75f;

            // Points for 'A' (top, bottom-left, bottom-right, crossbar-left, crossbar-right)
            QPointF p_top(centerX, bottomY - charHeight);
            QPointF p_bl(centerX - charWidth / 2.0f, bottomY);
            QPointF p_br(centerX + charWidth / 2.0f, bottomY);

            float crossbarHeightRatio = 0.4f; // crossbar is 40% up from baseline
            QPointF p_cl(centerX - charWidth / 2.0f * (1.0f - crossbarHeightRatio),
                           bottomY - charHeight * crossbarHeightRatio);
            QPointF p_cr(centerX + charWidth / 2.0f * (1.0f - crossbarHeightRatio),
                           bottomY - charHeight * crossbarHeightRatio);

            // Use m_centralColumnHeight as a progress factor (0.0 to height()/2.0)
            // Normalize this progress: currentHeight / (splashHeight/2.0)
            // The animation sets m_centralColumnHeight's EndValue to height() / 2.0
            float fullProgress = m_centralColumnHeight / (height() / 2.0f);
            fullProgress = qBound(0.0f, fullProgress, 1.0f); // Clamp between 0 and 1

            // Draw parts of 'A' based on fullProgress
            // Part 1: Left leg (0.0 to 0.4 progress)
            if (fullProgress > 0) {
                float part1Progress = qBound(0.0f, fullProgress / 0.4f, 1.0f);
                painter.drawLine(p_top, p_top + (p_bl - p_top) * part1Progress);
            }

            // Part 2: Right leg (drawn from 0.2 to 0.7 progress)
            if (fullProgress > 0.2f) {
                float part2Progress = qBound(0.0f, (fullProgress - 0.2f) / 0.5f, 1.0f);
                painter.drawLine(p_top, p_top + (p_br - p_top) * part2Progress);
            }

            // Part 3: Crossbar (drawn from 0.5 to 1.0 progress)
            if (fullProgress > 0.5f) {
                float part3Progress = qBound(0.0f, (fullProgress - 0.5f) / 0.5f, 1.0f);
                painter.drawLine(p_cl, p_cl + (p_cr - p_cl) * part3Progress);
            }
        }
    }
}
