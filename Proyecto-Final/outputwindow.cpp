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
    for(int r = 0; r < analysis.minutiae.rows; r++)
    {
        if(analysis.minutiae.at<float>(r,2) == fp::ENDING)
        {
            terminaciones++;
        }
        if(analysis.minutiae.at<float>(r,2) == fp::BIFURCATION)
        {
            bifurcaciones++;
        }
    }
//    for(const cv::KeyPoint &kp : analysis.keypoints)
//    {
//        if((int)kp.class_id == fp::ENDING)
//        {
//            terminaciones++;
//        }
//        if((int)kp.class_id == fp::BIFURCATION)
//        {
//            bifurcaciones++;
//        }
//    }
    for(const cv::KeyPoint &kp : analysis.l1_features)
    {
        if((int)kp.class_id == fp::DELTA)
        {
            deltas++;
        }
        if((int)kp.class_id == fp::LOOP)
        {
            cores++;
        }
    }
    deltas = round(deltas / 4.0);
    cores = round(cores / 4.0);
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
