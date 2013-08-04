#ifndef REPROPERTIESDIALOG_H
#define REPROPERTIESDIALOG_H

#include <QDialog>

class REDocumentView;
class REScoreController;
class RETrack;
class REScoreSettings;
class REScore;

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

    void SetActiveTrack(int trackIndex);

protected slots:
    void on_addTrackButton_clicked();
    void on_deleteTrackButton_clicked();
    void on_nameText_editingFinished();
    void on_shortNameText_editingFinished();
    void on_midiProgramList_currentRowChanged(int);
    void on_trackList_trackSelected(int trackIndex);
    void on_tuningButton_clicked();

    void on_addPartButton_clicked();
    void on_deletePartButton_clicked();
    void on_partNameText_editingFinished();
    void on_partList_partSelected(int);
    void on_showStandardNotation_toggled(bool);
    void on_showTablature_toggled(bool);
    void on_useMultibarRests_toggled(bool);
    void on_hideDynamics_toggled(bool);

protected slots:
    void trackListModelAboutToBeReset();
    void trackListModelReset();
    void partListModelAboutToBeReset();
    void partListModelReset();

private:
    void InitializeMidiTable();
    void InitializeFromDocument();
    void Cleanup();
    void UpdateTrackUI(const RETrack* track);
    void UpdatePartUI(const REScoreSettings* scoreSettings);

private:
    Ui::REPropertiesDialog *ui;
    int _backupTrackIndex;
    int _backupPartIndex;
};

#endif // REPROPERTIESDIALOG_H
