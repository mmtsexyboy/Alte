#include "splashscreen.h"
#include <QApplication> // For QScreen, if needed, though not directly here now
#include <QScreen>      // For screen geometry
#include <QAnimationGroup>
#include <QFontMetrics>

#include <QParallelAnimationGroup> // Ensure this is included

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent),
      m_centralColumnHeight(0.0),
      m_centralColumnOpacity(0.0),
      m_columnHeightAnimation(nullptr),
      m_columnOpacityAnimation(nullptr),
      m_centralColumnAnimationGroup(nullptr)
{
    setFixedSize(400, 300);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    // Setup for the new animation
    m_centralColumnAnimationGroup = new QParallelAnimationGroup(this);

    m_columnHeightAnimation = new QPropertyAnimation(this, "centralColumnHeight", this);
    // Duration will be set in startCentralColumnAnimation
    m_columnHeightAnimation->setEasingCurve(QEasingCurve::OutQuad);

    m_columnOpacityAnimation = new QPropertyAnimation(this, "centralColumnOpacity", this);
    // Duration will be set in startCentralColumnAnimation
    m_columnOpacityAnimation->setEasingCurve(QEasingCurve::InQuad); // Fade in

    m_centralColumnAnimationGroup->addAnimation(m_columnHeightAnimation);
    m_centralColumnAnimationGroup->addAnimation(m_columnOpacityAnimation);

    connect(m_centralColumnAnimationGroup, &QParallelAnimationGroup::finished, this, &SplashScreen::animationFinished);
}

SplashScreen::~SplashScreen()
{
    // m_centralColumnAnimationGroup is a child of 'this' and will be deleted.
    // m_columnHeightAnimation and m_columnOpacityAnimation are children of m_centralColumnAnimationGroup.
}

void SplashScreen::startCentralColumnAnimation(int durationMs) {
    m_columnHeightAnimation->setDuration(durationMs);
    m_columnHeightAnimation->setStartValue(0.0);
    m_columnHeightAnimation->setEndValue(height() * 0.80); // Grow to 80% of splash screen height

    m_columnOpacityAnimation->setDuration(durationMs); // Same duration for simultaneous effect
    m_columnOpacityAnimation->setStartValue(0.0);
    m_columnOpacityAnimation->setEndValue(1.0);

    m_centralColumnAnimationGroup->start();
}

void SplashScreen::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background (darker, less transparent for better contrast with column)
    painter.fillRect(rect(), QColor(20, 20, 30, 230));

    if (m_centralColumnHeight > 0 && m_centralColumnOpacity > 0) {
        qreal columnWidth = 50.0;
        QRectF columnRect( (width() - columnWidth) / 2.0,
                           height() - m_centralColumnHeight, // Anchored at bottom
                           columnWidth,
                           m_centralColumnHeight );

        QLinearGradient gradient(columnRect.topLeft(), columnRect.bottomLeft());
        // Hardcoded cyberPulse color for now: #00c7a4
        QColor cyberPulse(0, 199, 164);
        QColor cyberPulseDarker = cyberPulse.darker(150);

        gradient.setColorAt(0.0, cyberPulse); // Top (tip of the column)
        gradient.setColorAt(1.0, cyberPulseDarker);   // Bottom

        painter.setOpacity(m_centralColumnOpacity);
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen); // No border for the column itself

        // Simple rectangle for now. Can be replaced with a more complex shape or SVG later.
        painter.drawRect(columnRect);
    }
}
