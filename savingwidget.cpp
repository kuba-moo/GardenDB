#include "database.h"
#include "savingwidget.h"
#include "savingthread.h"
#include "ui_savingwidget.h"

SavingWidget::SavingWidget(Database *db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SavingWidget)
{
    ui->setupUi(this);

    if (!db->isModified()) {
        ui->progressBar->setVisible(false);
        ui->msg->setVisible(false);
        ui->header->setText(trUtf8("No changes needs to be saved"));
    }

    ui->backToTable->setVisible(false);
    connect(ui->backToTable, SIGNAL(clicked()), SIGNAL(finished()));

    thread = new SavingThread(db, this);
    connect(thread, SIGNAL(finished()), SLOT(threadDone()));
    connect(thread, SIGNAL(advance(int)), SLOT(advance(int)));
    connect(thread, SIGNAL(advance(int,QString)), SLOT(advance(int,QString)));
    thread->start();
}

SavingWidget::~SavingWidget()
{
    delete ui;
}

void SavingWidget::threadDone()
{
    ui->progressBar->setValue(100);
    ui->msg->setText(trUtf8("Done"));
    ui->backToTable->setVisible(true);

    emit done();
}

void SavingWidget::advance(int percent, const QString &msg)
{
    ui->progressBar->setValue(percent);
    if (msg.length())
        ui->msg->setText(msg);
}
