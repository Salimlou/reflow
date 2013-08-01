#include "RECreateTrackDialog.h"
#include "ui_RECreateTrackDialog.h"

#include "REUndoCommand.h"

RECreateTrackDialog::RECreateTrackDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RECreateTrackDialog)
{
    ui->setupUi(this);
    InitializeTemplates();
}

RECreateTrackDialog::~RECreateTrackDialog()
{
    delete ui;
}

bool RECreateTrackDialog::AddToFirstPartChecked() const
{
    return ui->addToFirstPartCheckbox->checkState() == Qt::Checked;
}

bool RECreateTrackDialog::CreatePartChecked() const
{
    return ui->createPartCheckbox->checkState() == Qt::Checked;
}

void RECreateTrackDialog::InitializeTemplates()
{
    // Acoustic Guitar
    {
        RECreateTrackOptions opts;
        opts.name = "Acoustic Guitar";
        opts.shortName = "Gtr.";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::GuitarInstrument;
        int tuning[] = {64, 59, 55, 50, 45, 40};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 24;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }

    // Electric Guitar
    {
        RECreateTrackOptions opts;
        opts.name = "Electric Guitar";
        opts.shortName = "Gtr.";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::GuitarInstrument;
        int tuning[] = {64, 59, 55, 50, 45, 40};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 27;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }

    // Electric Guitar (7 strings)
    {
        RECreateTrackOptions opts;
        opts.name = "Electric Guitar (7 Strings)";
        opts.shortName = "Gtr.";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::GuitarInstrument;
        int tuning[] = {64, 59, 55, 50, 45, 40, 35};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 27;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }


    // Electric Bass
    {
        RECreateTrackOptions opts;
        opts.name = "Electric Bass";
        opts.shortName = "Bass";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::BassInstrument;
        int tuning[] = {43, 38, 33, 28};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 34;
        opts.firstClef = opts.secondClef = Reflow::BassClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }

    // Electric Bass (5 strings)
    {
        RECreateTrackOptions opts;
        opts.name = "Electric Bass (5 Strings)";
        opts.shortName = "Bass";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::BassInstrument;
        int tuning[] = {43, 38, 33, 28, 23};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 34;
        opts.firstClef = opts.secondClef = Reflow::BassClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }

    // Electric Bass (6 strings)
    {
        RECreateTrackOptions opts;
        opts.name = "Electric Bass (6 Strings)";
        opts.shortName = "Bass";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::BassInstrument;
        int tuning[] = {48, 43, 38, 33, 28, 23};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 34;
        opts.firstClef = opts.secondClef = Reflow::BassClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::Ottavia_8vb;
        _templates.push_back(opts);
    }

    // Banjo
    {
        RECreateTrackOptions opts;
        opts.name = "Banjo";
        opts.shortName = "Bnj.";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::BanjoInstrument;
        int tuning[] = {64, 57, 50, 43};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 105;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    // Banjo (5 Strings)
    {
        RECreateTrackOptions opts;
        opts.name = "Banjo (5 Strings)";
        opts.shortName = "Bnj.";
        opts.droneString = true;
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::BanjoInstrument;
        int tuning[] = {62, 59, 55, 50, 67};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 105;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    // Ukulele
    {
        RECreateTrackOptions opts;
        opts.name = "Ukulele";
        opts.shortName = "Uk.";
        opts.type = Reflow::TablatureTrack;
        opts.subType = Reflow::UkuleleInstrument;
        int tuning[] = {69, 64, 60, 67};
        for(int t : tuning) {opts.tuning.push_back(t);}
        opts.midiProgram = 24;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    // Drumkit
    {
        RECreateTrackOptions opts;
        opts.name = "Drumkit";
        opts.shortName = "Dr.";
        opts.type = Reflow::DrumsTrack;
        opts.midiProgram = 0;
        opts.firstClef = opts.secondClef = Reflow::NeutralClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    // Standard
    {
        RECreateTrackOptions opts;
        opts.name = "Keyboard";
        opts.shortName = "Inst.";
        opts.type = Reflow::StandardTrack;
        opts.midiProgram = 0;
        opts.firstClef = opts.secondClef = Reflow::TrebleClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    // Standard (Grand Staff)
    {
        RECreateTrackOptions opts;
        opts.name = "Keyboard (Grand Staff)";
        opts.shortName = "Inst.";
        opts.type = Reflow::StandardTrack;
        opts.useGrandStaff = true;
        opts.midiProgram = 0;
        opts.firstClef = Reflow::TrebleClef;
        opts.secondClef = Reflow::BassClef;
        opts.firstOttavia = opts.secondOttavia = Reflow::NoOttavia;
        _templates.push_back(opts);
    }

    for(const RECreateTrackOptions& opts : _templates) {
        ui->trackTemplateList->addItem(QString::fromStdString(opts.name));
    }
    ui->trackTemplateList->setCurrentRow(0);
}

void RECreateTrackDialog::on_trackTemplateList_itemDoubleClicked(QListWidgetItem * item)
{
    done(1);
}

const RECreateTrackOptions& RECreateTrackDialog::SelectedTrackTemplate() const
{
    int row = ui->trackTemplateList->currentRow();
    if(row < 0) row = 0;

    return _templates[row];
}
