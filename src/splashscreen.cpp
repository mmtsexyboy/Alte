#include "splashscreen.h"
#include <QApplication>
#include <QScreen>
#include <QAnimationGroup>
#include <QPainterPath>
#include <QDebug>
#include <QFontMetrics>

#include <QParallelAnimationGroup>

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent),
      m_centralColumnHeight(0.0),
      m_centralColumnOpacity(0.0),
      m_columnHeightAnimation(nullptr),
      m_columnOpacityAnimation(nullptr),
      m_centralColumnAnimationGroup(nullptr),
      m_glyphAnimationStarted(false)
{
    setFixedSize(480, 360);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
    setAttribute(Qt::WA_TranslucentBackground);

    m_centralColumnAnimationGroup = new QParallelAnimationGroup(this);

    m_columnHeightAnimation = new QPropertyAnimation(this, "centralColumnHeight", this);
    m_columnHeightAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_columnOpacityAnimation = new QPropertyAnimation(this, "centralColumnOpacity", this);
    m_columnOpacityAnimation->setEasingCurve(QEasingCurve::Linear);

    m_centralColumnAnimationGroup->addAnimation(m_columnHeightAnimation);
    m_centralColumnAnimationGroup->addAnimation(m_columnOpacityAnimation);

    connect(m_centralColumnAnimationGroup, &QParallelAnimationGroup::finished, this, &SplashScreen::animationFinished);
}

SplashScreen::~SplashScreen()
{
}

void SplashScreen::startGlyphAnimation(int durationMs) {
    m_glyphAnimationStarted = true;

    m_columnHeightAnimation->setDuration(durationMs);
    m_columnHeightAnimation->setStartValue(0.0);
    m_columnHeightAnimation->setEndValue(height() / 2.0);

    m_columnOpacityAnimation->setDuration(durationMs * 0.7);
    m_columnOpacityAnimation->setStartValue(0.0);
    m_columnOpacityAnimation->setEndValue(1.0);

    qDebug() << "Starting glyph animation";
    m_centralColumnAnimationGroup->start();
    update();
}

void SplashScreen::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!m_glyphAnimationStarted) {
        painter.fillRect(rect(), QColor(0, 0, 0));
    } else {
        painter.fillRect(rect(), QColor(0, 0, 0));

        if (m_centralColumnHeight > 0 && m_centralColumnOpacity > 0) {
            painter.setOpacity(m_centralColumnOpacity);
            QColor neonColor(0, 199, 164);

            float fullProgress = m_centralColumnHeight / (height() / 2.0f);
            fullProgress = qBound(0.0f, fullProgress, 1.0f);

            int currentStrokeWidth = 2 + static_cast<int>(fullProgress * 4);
            painter.setPen(QPen(neonColor, currentStrokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

            float centerX = width() / 2.0f;
            float bottomY = height() * 0.75f;
            float charHeight = height() * 0.4f;
            float charWidth = charHeight * 0.75f;

            QPointF p_top(centerX, bottomY - charHeight);
            QPointF p_bl(centerX - charWidth / 2.0f, bottomY);
            QPointF p_br(centerX + charWidth / 2.0f, bottomY);

            float crossbarHeightRatio = 0.4f;
            QPointF p_cl(centerX - charWidth / 2.0f * (1.0f - crossbarHeightRatio),
                           bottomY - charHeight * crossbarHeightRatio);
            QPointF p_cr(centerX + charWidth / 2.0f * (1.0f - crossbarHeightRatio),
                           bottomY - charHeight * crossbarHeightRatio);

            if (fullProgress > 0) {
                float part1Progress = qBound(0.0f, fullProgress / 0.4f, 1.0f);
                if (part1Progress > 0) {
                    QPainterPath leftLegPath;
                    leftLegPath.moveTo(p_top);
                    QPointF control_p1_bl(p_top.x() - charWidth * 0.25f, p_top.y() + (p_bl.y() - p_top.y()) * 0.5f);
                    QPointF target_p_bl = p_top + (p_bl - p_top) * part1Progress;

                    QPointF animated_control_p1_bl = p_top + (control_p1_bl - p_top) * part1Progress;

                    leftLegPath.quadTo(animated_control_p1_bl, target_p_bl);
                    painter.drawPath(leftLegPath);
                }
            }

            if (fullProgress > 0.2f) {
                float part2Progress = qBound(0.0f, (fullProgress - 0.2f) / 0.5f, 1.0f);
                if (part2Progress > 0) {
                    QPainterPath rightLegPath;
                    rightLegPath.moveTo(p_top);
                    QPointF control_p1_br(p_top.x() + charWidth * 0.25f, p_top.y() + (p_br.y() - p_top.y()) * 0.5f);
                    QPointF target_p_br = p_top + (p_br - p_top) * part2Progress;
                    QPointF animated_control_p1_br = p_top + (control_p1_br - p_top) * part2Progress;

                    rightLegPath.quadTo(animated_control_p1_br, target_p_br);
                    painter.drawPath(rightLegPath);
                }
            }

            if (fullProgress > 0.5f) {
                float part3Progress = qBound(0.0f, (fullProgress - 0.5f) / 0.5f, 1.0f);
                if (part3Progress > 0) {
                    painter.drawLine(p_cl, p_cl + (p_cr - p_cl) * part3Progress);
                }
            }
        }
    }
}
