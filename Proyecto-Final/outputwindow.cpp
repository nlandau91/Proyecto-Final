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

void OutputWindow::setup(const fp::Analysis &analysis, const cv::Mat &output)
{
    int descriptores = analysis.descriptors.rows;
    int terminaciones = 0;
    int bifurcaciones = 0;
    int deltas = 0;
    int cores = 0;
    int whorls = 0;
    QString type = "tba";
    for(cv::KeyPoint kp : analysis.l2_features)
    {
        if(kp.size == 1)
        {
            terminaciones++;
        }
        if(kp.size == 3)
        {
            bifurcaciones++;
        }
    }
    for(cv::KeyPoint kp : analysis.l1_features)
    {
        if(kp.size == fp::DELTA)
        {
            deltas++;
        }
        if(kp.size == fp::LOOP)
        {
            cores++;
        }
    }
    deltas = deltas / 4;
    cores = cores / 4;
    if(deltas == 0 && cores == 0)
    {
        type = "Arch";
    }
    if(cores == 2 || whorls == 1)
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
