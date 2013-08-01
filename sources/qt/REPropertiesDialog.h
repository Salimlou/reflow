#ifndef REPROPERTIESDIALOG_H
#define REPROPERTIESDIALOG_H

#include <QDialog>

class REDocumentView;
class REScoreController;
class RETrack;

namespace Ui {
class REPropertiesDialog;
}

class REPropertiesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REPropertiesDialog(REDocumentView *parent = 0);
    ~REPropertiesDialog();
    
public:
    REDocumentView* DocumentView();
    REScoreController* ScoreController();

protected slots:
    void on_addTrackButton_clicked();
    void on_deleteTrackButton_clicked();
    void on_nameText_editingFinished();
    void on_shortNameText_editingFinished();
    void on_midiProgramList_currentRowChanged(int);
    void on_trackList_trackSelected(int trackIndex);
    void on_tuningButton_clicked();

protected slots:
    void trackListModelAboutToBeReset();
    void trackListModelReset();

private:
    void InitializeMidiTable();
    void InitializeFromDocument();
    void Cleanup();
    void UpdateTrackUI(const RETrack* track);

private:
    Ui::REPropertiesDialog *ui;
    int _backupTrackIndex;
};

#endif // REPROPERTIESDIALOG_H
