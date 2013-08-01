#ifndef REFILEPROPERTIESDIALOG_H
#define REFILEPROPERTIESDIALOG_H

#include <QDialog>

class RESong;

namespace Ui {
class REFilePropertiesDialog;
}

class REFilePropertiesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit REFilePropertiesDialog(QWidget *parent = 0);
    ~REFilePropertiesDialog();

    void Initialize(const RESong* song);
    
    QString Title() const;
    QString Subtitle() const;
    QString Artist() const;
    QString Album() const;
    QString MusicBy() const;
    QString LyricsBy() const;
    QString Transcriber() const;
    QString Copyright() const;

private:
    Ui::REFilePropertiesDialog *ui;
};

#endif // REFILEPROPERTIESDIALOG_H
