#ifndef REKEYSIGNATUREPREVIEW_H
#define REKEYSIGNATUREPREVIEW_H

#include <QWidget>

#include "RETypes.h"

class REKeySignaturePreview : public QWidget
{
    Q_OBJECT
public:
    explicit REKeySignaturePreview(QWidget *parent = 0);
    
    void SetKeySignature(const REKeySignature& sig);
    const REKeySignature& KeySignature() const {return _keySignature;}

    float YOffsetOfLine(int lineIndex) const;

protected:
    virtual void paintEvent(QPaintEvent *);
    
private:
    REKeySignature _keySignature;
};

#endif // REKEYSIGNATUREPREVIEW_H
