#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include <QString>
#include <QImage>
#include <QPainter>
#include <QFontMetrics>
#include "displayobject.h"

class TextObject : public DisplayObject
{
public:
    TextObject(QString displayString, QObject* parent=0);
    ~TextObject();

    void loadImage(unsigned int idx);

    int getPixelValue(int x, int y) { return -1; }

    void setDuration(int durationSeconds)
    {
        setFrameRate(1.0);
        setNumFrames(durationSeconds);
    }
    int duration() { return p_numFrames; }

    void setText(QString text) {p_TextString=text;};
    void setFont(QFont font) {p_TextFont=font;}

    QString text() {return p_TextString;};
    QFont font() {return p_TextFont;};

public slots:

    void refreshDisplayImage() {return;}
private:

    void drawText();

    QString p_TextString;
    QFont p_TextFont;

};

#endif // TEXTOBJECT_H