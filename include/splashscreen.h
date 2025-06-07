#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QTimer>

class SplashScreen : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal centralColumnHeight READ centralColumnHeight WRITE setCentralColumnHeight)
    Q_PROPERTY(qreal centralColumnOpacity READ centralColumnOpacity WRITE setCentralColumnOpacity)

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

    void startGlyphAnimation(int durationMs);

    qreal centralColumnHeight() const { return m_centralColumnHeight; }
    void setCentralColumnHeight(qreal height) { m_centralColumnHeight = height; update(); }

    qreal centralColumnOpacity() const { return m_centralColumnOpacity; }
    void setCentralColumnOpacity(qreal opacity) { m_centralColumnOpacity = opacity; update(); }

signals:
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal m_centralColumnHeight;
    qreal m_centralColumnOpacity;

    QPropertyAnimation *m_columnHeightAnimation;
    QPropertyAnimation *m_columnOpacityAnimation;
    QAnimationGroup *m_centralColumnAnimationGroup;

    bool m_glyphAnimationStarted;

};

#endif // SPLASHSCREEN_H
