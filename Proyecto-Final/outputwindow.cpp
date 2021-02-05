#include "outputwindow.h"
#include "ui_outputwindow.h"

OutputWindow::OutputWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OutputWindow)
{
    ui->setupUi(this);
}

OutputWindow::~OutputWindow()
{
    delete ui;
}

void OutputWindow::setup(int cores, int deltas, QString clasificacion, int terminaciones, int bifurcaciones)
{
    ui->lbl_cores->setText(QString::number(cores));
    ui->lbl_deltas->setText(QString::number(deltas));
    ui->lbl_clas->setText(clasificacion);
    ui->lbl_term->setText(QString::number(terminaciones));
    ui->lbl_bif->setText(QString::number(bifurcaciones));
}
