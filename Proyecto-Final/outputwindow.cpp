#include "outputwindow.h"
#include "ui_outputwindow.h"
#include "appsettings.h"
#include <opencv2/imgproc.hpp>

#include <QFileDialog>

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

void OutputWindow::setup(const fp::FingerprintTemplate &fp_template, const fp::Preprocessed &prep)
{
    this->fp_template = fp_template;
    this->prep = prep;
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
        type = "Loop/Tented Arch";
    }

    ui->lbl_cores->setText(QString::number(cores));
    ui->lbl_deltas->setText(QString::number(deltas));
    ui->lbl_clas->setText(type);
    ui->lbl_term->setText(QString::number(terminaciones));
    ui->lbl_bif->setText(QString::number(bifurcaciones));
    ui->lbl_des->setText(QString::number(descriptores));

    selected = prep.thinned;
    drawover = true;
    segment = true;
    set_output();
}

void OutputWindow::set_output()
{
    cv::Mat output;
    selected.convertTo(output,CV_8UC1);
    if(segment)
    {
        output.setTo(255,prep.roi==0);
    }
    cv::cvtColor(output,output,cv::COLOR_GRAY2BGR);

    if(drawover)
    {
        output = fp::draw_keypoints(output,fp_template.minutiaes);
        output = fp::draw_keypoints(output,fp_template.keypoints);
        output = fp::draw_singularities(output,fp_template.singularities);
    }
    ui->lbl_output->setPixmap(fp::cv_mat_to_qpixmap(output));
}

void OutputWindow::on_radioButton_filtered_clicked()
{
    selected = prep.filtered;
    set_output();
}

void OutputWindow::on_radioButton_thinned_clicked()
{
    selected = prep.thinned;
    set_output();
}

void OutputWindow::on_checkBox_marks_stateChanged(int arg1)
{
    drawover = arg1 == 2;
    set_output();
}

void OutputWindow::on_radioButton_orientation_clicked()
{
    selected = fp::visualize_angles(prep.result,prep.orientation);
    set_output();
}

void OutputWindow::on_radioButton_normalized_clicked()
{
    selected = prep.normalized;
    set_output();
}

void OutputWindow::on_radioButton_original_clicked()
{
    selected = prep.original;
    set_output();
}

void OutputWindow::on_checkBox_segment_stateChanged(int arg1)
{
    segment = arg1 == 2;
    set_output();
}

void OutputWindow::on_pushButton_guardar_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "",
                               tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    ui->lbl_output->pixmap()->save(fileName);
}
