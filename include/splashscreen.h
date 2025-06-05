#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QPropertyAnimation> // For animations
#include <QPainter>
#include <QTimer>

class SplashScreen : public QWidget
{
    Q_OBJECT
    // Properties for the new central column animation
    Q_PROPERTY(qreal centralColumnHeight READ centralColumnHeight WRITE setCentralColumnHeight)
    Q_PROPERTY(qreal centralColumnOpacity READ centralColumnOpacity WRITE setCentralColumnOpacity)

public:
    explicit SplashScreen(QWidget *parent = nullptr); // ThemeManager can be passed here if needed later
    ~SplashScreen();

    // void startAnimation(int durationMs); // Old animation starter, will be replaced
    void startCentralColumnAnimation(int durationMs); // New animation starter

    // Accessors for new properties
    qreal centralColumnHeight() const { return m_centralColumnHeight; }
    void setCentralColumnHeight(qreal height) { m_centralColumnHeight = height; update(); }

    qreal centralColumnOpacity() const { return m_centralColumnOpacity; }
    void setCentralColumnOpacity(qreal opacity) { m_centralColumnOpacity = opacity; update(); }

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // Members for central column animation
    qreal m_centralColumnHeight;
    qreal m_centralColumnOpacity;

    QPropertyAnimation *m_columnHeightAnimation;
    QPropertyAnimation *m_columnOpacityAnimation;
    QAnimationGroup *m_centralColumnAnimationGroup;

    // Comment out or remove old animation members
    // qreal m_pulseRadius;
    // qreal m_pulseOpacity;
    // qreal m_textOpacity;

    // QPropertyAnimation *m_radiusAnimation;
    // QPropertyAnimation *m_opacityAnimation;
    // QPropertyAnimation *m_textOpacityAnimation;
    // QAnimationGroup *m_animationGroup; // Will be repurposed or replaced by m_centralColumnAnimationGroup
};

#endif // SPLASHSCREEN_H
