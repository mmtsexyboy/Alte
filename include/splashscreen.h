#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QPropertyAnimation> // For animations
#include <QPainter>
// Removed QTimer, QList, QRectF as particles are removed
#include <QPixmap> // Added for icon

// Particle structure removed

class SplashScreen : public QWidget
{
    Q_OBJECT
    // Properties for central column and icon animation
    Q_PROPERTY(qreal centralColumnHeight READ centralColumnHeight WRITE setCentralColumnHeight)
    Q_PROPERTY(qreal centralColumnOpacity READ centralColumnOpacity WRITE setCentralColumnOpacity)
    Q_PROPERTY(qreal iconOpacity READ iconOpacity WRITE setIconOpacity)

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen(); // Keep destructor for potential future cleanup, even if empty now

    void startCentralColumnAnimation(int durationMs);

    // Accessors for properties
    qreal centralColumnHeight() const { return m_centralColumnHeight; }
    void setCentralColumnHeight(qreal height) { m_centralColumnHeight = height; update(); }

    qreal centralColumnOpacity() const { return m_centralColumnOpacity; }
    void setCentralColumnOpacity(qreal opacity) { m_centralColumnOpacity = opacity; update(); }

    qreal iconOpacity() const { return m_iconOpacity; }
    void setIconOpacity(qreal opacity) { m_iconOpacity = opacity; update(); }

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

// private slots: // Removed as updateParticles is removed
    // void updateParticles();

private:
    // Members for central column and icon animation
    qreal m_centralColumnHeight;
    qreal m_centralColumnOpacity;
    qreal m_iconOpacity;

    QPropertyAnimation *m_columnHeightAnimation;
    QPropertyAnimation *m_columnOpacityAnimation;
    QPropertyAnimation *m_iconOpacityAnimation;
    QAnimationGroup *m_centralColumnAnimationGroup;

    QPixmap m_iconPixmap;

    // Particle animation members removed
    // QList<Particle> m_particles;
    // QTimer *m_particleTimer;

    // Old animation members (already commented, now fully removed)
    // qreal m_pulseRadius;
    // qreal m_pulseOpacity;
    // qreal m_textOpacity;
    // QPropertyAnimation *m_radiusAnimation;
    // QPropertyAnimation *m_opacityAnimation;
    // QPropertyAnimation *m_textOpacityAnimation;
    // QAnimationGroup *m_animationGroup;
};

#endif // SPLASHSCREEN_H
