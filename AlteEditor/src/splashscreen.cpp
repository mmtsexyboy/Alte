#include "splashscreen.h"
#include <QApplication> // For QScreen, if needed, though not directly here now
#include <QScreen>      // For screen geometry
#include <QAnimationGroup>
#include <QFontMetrics>
#include <QPainterPath> // Added for rounded corners
#include <QDebug>       // Added for qWarning
// QRandomGenerator and QTimer removed as particles are removed

#include <QParallelAnimationGroup> // Ensure this is included

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent),
      m_centralColumnHeight(0.0),
      m_centralColumnOpacity(0.0),
      m_iconOpacity(0.0), // Initialized icon opacity
      m_columnHeightAnimation(nullptr),
      m_columnOpacityAnimation(nullptr),
      m_iconOpacityAnimation(nullptr), // Initialized icon animation member
      m_centralColumnAnimationGroup(nullptr)
      // m_particleTimer initialization removed
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

    // Icon initialization
    if (!m_iconPixmap.load(":/icons/alte_icon.png")) {
        qWarning() << "Failed to load splash screen icon from resources: :/icons/alte_icon.png";
    }
    m_iconOpacityAnimation = new QPropertyAnimation(this, "iconOpacity", this);
    m_centralColumnAnimationGroup->addAnimation(m_iconOpacityAnimation);


    connect(m_centralColumnAnimationGroup, &QParallelAnimationGroup::finished, this, &SplashScreen::animationFinished);

    // Particle initialization code removed
}

SplashScreen::~SplashScreen()
{
    // Particle timer stop call removed
    // m_centralColumnAnimationGroup is a child of 'this' and will be deleted.
    // Other animations are children of m_centralColumnAnimationGroup.
}

// updateParticles() method removed

void SplashScreen::startCentralColumnAnimation(int durationMs) {
    m_columnHeightAnimation->setDuration(durationMs);
    m_columnHeightAnimation->setStartValue(0.0);
    m_columnHeightAnimation->setEndValue(height() * 0.80); // Grow to 80% of splash screen height

    m_columnOpacityAnimation->setDuration(durationMs); // Same duration for simultaneous effect
    m_columnOpacityAnimation->setStartValue(0.0);
    m_columnOpacityAnimation->setEndValue(1.0);

    // Configure icon opacity animation
    m_iconOpacityAnimation->setDuration(durationMs * 0.8); // Fades in over 80% of the column animation time
    m_iconOpacityAnimation->setStartValue(0.0);
    m_iconOpacityAnimation->setEndValue(1.0);
    m_iconOpacityAnimation->setEasingCurve(QEasingCurve::InQuad);

    m_centralColumnAnimationGroup->start();
    // Particle timer start call removed
}

void SplashScreen::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(20, 20, 30, 230));

    // Draw Icon
    if (!m_iconPixmap.isNull() && m_iconOpacity > 0) {
        painter.save();
        painter.setOpacity(m_iconOpacity);
        qreal iconWidth = m_iconPixmap.width();
        qreal iconHeight = m_iconPixmap.height();
        qreal iconX = (width() - iconWidth) / 2.0;
        qreal iconY = (height() * 0.25) - (iconHeight / 2.0); // Centered around 25% of screen height
        painter.drawPixmap(iconX, iconY, m_iconPixmap);
        painter.restore();
    }

    // Particle drawing code removed

    // Draw Central Column
    // The painter's opacity should be 1.0 here due to painter.restore() after icon drawing,
    // or if icon wasn't drawn. If icon was drawn and painter.restore() wasn't used,
    // this would need painter.setOpacity(1.0) before setting column specific opacity.
    // Assuming icon drawing correctly uses save/restore.
    if (m_centralColumnHeight > 0 && m_centralColumnOpacity > 0) {
        qreal columnWidth = 50.0;
        QRectF columnRect( (width() - columnWidth) / 2.0,
                           height() - m_centralColumnHeight, // Anchored at bottom
                           columnWidth,
                           m_centralColumnHeight );

        QLinearGradient gradient(columnRect.topLeft(), columnRect.bottomLeft());
        QColor cyberPulse(0, 199, 164);
        QColor cyberPulseLighter = cyberPulse.lighter(120);
        QColor cyberPulseDarker = cyberPulse.darker(150);

        gradient.setColorAt(0.0, cyberPulse);
        gradient.setColorAt(0.5, cyberPulseLighter);
        gradient.setColorAt(1.0, cyberPulseDarker);

        painter.setOpacity(m_centralColumnOpacity); // Apply column's own animated opacity
        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);

        QPainterPath path;
        path.addRoundedRect(columnRect, 10.0, 10.0);
        painter.drawPath(path);
    }
}
