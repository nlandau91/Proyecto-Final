#include "configdialog.h"
#include "ui_configdialog.h"
#include <QFile>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{

    ui->setupUi(this);

    //preprocessing
    ui->comboBox_thi->addItem("zhangsuen");
    ui->comboBox_thi->addItem("guohall");

    //analizing
    ui->comboBox_kp->addItem("harris");
    ui->comboBox_kp->addItem("shitomasi");
    ui->comboBox_kp->addItem("surf");
    ui->comboBox_kp->addItem("cn");

    ui->comboBox_feat->addItem("orb");
    ui->comboBox_feat->addItem("surf");
    ui->comboBox_feat->addItem("sift");

    //matching
    ui->comboBox_ransacModel->addItem("similarity");
    ui->comboBox_ransacModel->addItem("affine");
    ui->comboBox_ransacModel->addItem("projective");

    load_settings();
}



ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::load_settings(bool def)
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    if(def || !QFile::exists(file))
    {
        file = QApplication::applicationDirPath()+"/default.ini";
    }
    QSettings settings(file, QSettings::IniFormat);

    //preprocessing

    ui->comboBox_thi->setCurrentText(settings.value("thinning_method").toString());
    ui->checkBox_seg->setChecked(settings.value("segment").toBool());
    ui->doubleSpinBox_roi->setValue(settings.value("roi_threshold").toDouble());
    ui->spinBox_orient->setValue(settings.value("blk_orient").toInt());
    ui->spinBox_freq->setValue(settings.value("blk_freq").toInt());
    ui->spinBox_segm->setValue(settings.value("blk_segm").toInt());
    ui->doubleSpinBox_kx->setValue(settings.value("gabor_kx").toDouble());
    ui->doubleSpinBox_ky->setValue(settings.value("gabor_ky").toDouble());

    //feature extraction
    ui->comboBox_kp->setCurrentText(settings.value("keypoint_detector").toString());
    ui->comboBox_feat->setCurrentText(settings.value("feature_extractor").toString());
    ui->spinBox_kpThresh->setValue(settings.value("keypoint_threshold").toInt());
    ui->checkBox_draw->setChecked(settings.value("draw_features").toBool());

    //feature matching
    ui->doubleSpinBox_matchThresh->setValue(settings.value("match_thresh").toDouble());
    ui->comboBox_ransacModel->setCurrentText(settings.value("ransac_model").toString());
    ui->doubleSpinBox_ransac->setValue(settings.value("ransac_threshold").toDouble());


}

void ConfigDialog::save_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    //preprocessing

    settings.setValue("thinning_method", ui->comboBox_thi->currentText());
    settings.setValue("segment",ui->checkBox_seg->isChecked());
    settings.setValue("roi_threshold", ui->doubleSpinBox_roi->value());
    settings.setValue("blk_orient",ui->spinBox_orient->value());
    settings.setValue("blk_freq",ui->spinBox_freq->value());
    settings.setValue("blk_segm",ui->spinBox_segm->value());
    settings.setValue("gabor_kx",ui->doubleSpinBox_kx->value());
    settings.setValue("gabor_ky",ui->doubleSpinBox_ky->value());

    //feature extraction
    settings.setValue("keypoint_detector", ui->comboBox_kp->currentText());
    settings.setValue("feature_extractor", ui->comboBox_feat->currentText());
    settings.setValue("keypoint_threshold", ui->spinBox_kpThresh->value());
    settings.setValue("draw_features",ui->checkBox_draw->isChecked());

    //feature matching
    settings.setValue("match_thresh", ui->doubleSpinBox_matchThresh->value());
    settings.setValue("ransac_model", ui->comboBox_ransacModel->currentText());
    settings.setValue("ransac_threshold", ui->doubleSpinBox_ransac->value());
}

void ConfigDialog::on_buttonBox_accepted()
{
    save_settings();
}

void ConfigDialog::on_btn_default_clicked()
{
    load_settings(true);
}

