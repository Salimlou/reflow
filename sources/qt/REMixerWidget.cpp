#include "REMixerWidget.h"
#include "REMixerRowWidget.h"
#include "REDocumentView.h"

#include "REScoreController.h"
#include "REPainter.h"
#include "RESong.h"
#include "RETrack.h"

#include <QPainter>

#include <sstream>

REMixerWidget::REMixerWidget(QWidget *parent) :
    QWidget(parent), _documentView(nullptr), _rowHeight(30.0)
{
}

void REMixerWidget::SetDocumentView(REDocumentView *doc)
{
    _documentView = doc;
    Refresh();
}

void REMixerWidget::Refresh()
{
    QObjectList widgets = children();
    qDeleteAll(widgets);

    if(_documentView == nullptr) return;

    REScoreController* scoreController = _documentView->ScoreController();
    const REScore* score = (scoreController ? scoreController->Score() : nullptr);
    const RESong* song = (score ? score->Song() : nullptr);

    int firstTrackIndex = 0;
    int lastTrackIndex = song->TrackCount()-1;

    for(int trackIndex=firstTrackIndex; trackIndex<=lastTrackIndex; ++trackIndex)
    {
        const RETrack* track = song->Track(trackIndex);
        bool presentInScore = (score->ContainsTrack(track));
        float rowY = trackIndex * _rowHeight;

        REMixerRowWidget* row = new REMixerRowWidget(this, trackIndex);
        row->move(0, rowY);
    }
}
