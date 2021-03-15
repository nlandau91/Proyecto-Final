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

void OutputWindow::setup(const fp::FingerprintTemplate &fp_template, const cv::Mat &output)
{
    int descriptores = fp_template.descriptors.rows;
    int terminaciones = 0;
    int bifurcaciones = 0;
    int deltas = 0;
    int cores = 0;
    int whorls = 0;
    QString type = "tba";
    for(const cv::KeyPoint &kp : fp_template.minutiaes)
    {
        if((int)kp.class_id == fp::ENDING)
        {
            terminaciones++;
        }
        if((int)kp.class_id == fp::BIFURCATION)
        {
            bifurcaciones++;
        }
    }
    for(const cv::KeyPoint &kp : fp_template.singularities)
    {
        if((int)kp.class_id == fp::DELTA)
        {
            deltas++;
        }
        if((int)kp.class_id == fp::LOOP)
        {
            cores++;
        }
        if((int)kp.class_id == fp::WHORL)
        {
            whorls++;
        }
    }
    deltas = round(deltas / 4.0);
    cores = round(cores / 4.0);
    if(deltas == 0 && cores == 0)
    {
        type = "Arch";
    }
    if(cores >= 2 || whorls >= 1)
    {
        type = "Whorl";
    }
    if(cores == 1)
    {
        type = "Loop";
    }

    ui->lbl_cores->setText(QString::number(cores));
    ui->lbl_deltas->setText(QString::number(deltas));
    ui->lbl_clas->setText(type);
    ui->lbl_term->setText(QString::number(terminaciones));
    ui->lbl_bif->setText(QString::number(bifurcaciones));
    ui->lbl_des->setText(QString::number(descriptores));

    ui->lbl_output->setPixmap(fp::cvMatToQPixmap(output));
}
