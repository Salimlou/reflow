#ifndef RECREATETRACKDIALOG_H
#define RECREATETRACKDIALOG_H

#include <QDialog>

#include "RESongController.h"

class QListWidgetItem;

namespace Ui {
class RECreateTrackDialog;
}

class RECreateTrackDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit RECreateTrackDialog(QWidget *parent = 0);
    ~RECreateTrackDialog();
    
    const RECreateTrackOptions& SelectedTrackTemplate() const;
    bool AddToFirstPartChecked() const;
    bool CreatePartChecked() const;

protected slots:
    void on_trackTemplateList_itemDoubleClicked(QListWidgetItem * item);

private:
    void InitializeTemplates();

private:
    Ui::RECreateTrackDialog *ui;
    RECreateTrackOptionsVector _templates;
};

#endif // RECREATETRACKDIALOG_H
