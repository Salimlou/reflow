#ifndef REMIXERWIDGET_H
#define REMIXERWIDGET_H

#include "RETypes.h"

#include <QWidget>

class REDocumentView;

class REMixerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit REMixerWidget(QWidget *parent = 0);
    
    void SetDocumentView(REDocumentView*);

    void Refresh();

    inline REDocumentView* DocumentView() {return _documentView;}

private:
    REDocumentView* _documentView;
    float _rowHeight;
};

#endif // REMIXERWIDGET_H
