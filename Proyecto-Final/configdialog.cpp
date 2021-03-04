#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{

    ui->setupUi(this);

    //preprocessing

    ui->comboBox_thi->addItem("none");
    ui->comboBox_thi->addItem("zhangsuen");
    ui->comboBox_thi->addItem("guohall");
    //ui->comboBox_thi->addItem("morph");

    ui->checkBox_seg->setChecked(true);

    //feature extraction
    //ui->comboBox_kp->addItem("harris");
    //ui->comboBox_kp->addItem("shitomasi");
    //ui->comboBox_kp->addItem("surf");
    ui->comboBox_kp->addItem("cn");

    ui->comboBox_feat->addItem("orb");
    ui->comboBox_feat->addItem("surf");

    ui->lineEdit_thresh->setText("120");

    ui->checkBox_draw->setChecked(true);

    //feature matching
    ui->lineEdit_match_thresh->setText("0.2");
    ui->checkBox_edgeMatching->setEnabled(true);
    ui->doubleSpinBox_edgeAngle->setValue(5.0);
    ui->doubleSpinBox_edgeDist->setValue(0.80);

    ui->spinBox_maps->setValue(8);
    ui->spinBox_segm->setValue(8);


    load_settings();
}



ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::load_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    //preprocessing

    ui->comboBox_thi->setCurrentText(settings.value("thinning_method").toString());
    ui->checkBox_seg->setChecked(settings.value("segment").toBool());
    ui->doubleSpinBox_roi->setValue(settings.value("roi_threshold").toDouble());
    ui->spinBox_maps->setValue(settings.value("blk_maps").toInt());
    ui->spinBox_segm->setValue(settings.value("blk_segm").toInt());
    ui->doubleSpinBox_kx->setValue(settings.value("gabor_kx").toDouble());
    ui->doubleSpinBox_ky->setValue(settings.value("gabor_ky").toDouble());

    //feature extraction
    ui->comboBox_kp->setCurrentText(settings.value("keypoint_extractor").toString());
    ui->comboBox_feat->setCurrentText(settings.value("feature_extractor").toString());
    ui->lineEdit_thresh->setText(settings.value("keypoint_threshold").toString());
    ui->checkBox_draw->setChecked(settings.value("draw_features").toBool());

    //feature matching
    ui->lineEdit_match_thresh->setText(settings.value("match_thresh").toString());
    ui->checkBox_edgeMatching->setChecked(settings.value("edge_matching").toBool());
    ui->doubleSpinBox_edgeAngle->setValue(settings.value("edge_angle").toDouble());
    ui->doubleSpinBox_edgeDist->setValue(settings.value("edge_dist").toDouble());


}

void ConfigDialog::save_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    //preprocessing

    settings.setValue("thinning_method", ui->comboBox_thi->currentText());
    settings.setValue("segment",ui->checkBox_seg->isChecked());
    settings.setValue("roi_threshold", ui->doubleSpinBox_roi->value());
    settings.setValue("blk_maps",ui->spinBox_maps->value());
    settings.setValue("blk_segm",ui->spinBox_segm->value());
    settings.setValue("gabor_kx",ui->doubleSpinBox_kx->value());
    settings.setValue("gabor_ky",ui->doubleSpinBox_ky->value());

    //feature extraction
    settings.setValue("keypoint_extractor", ui->comboBox_kp->currentText());
    settings.setValue("feature_extractor", ui->comboBox_feat->currentText());
    settings.setValue("keypoint_threshold", ui->lineEdit_thresh->text());
    settings.setValue("draw_features",ui->checkBox_draw->isChecked());

    //feature matching
    settings.setValue("match_thresh", ui->lineEdit_match_thresh->text());
    settings.setValue("edge_matching",ui->checkBox_edgeMatching->isChecked());
    settings.setValue("edge_angle", ui->doubleSpinBox_edgeAngle->value());
    settings.setValue("edge_dist", ui->doubleSpinBox_edgeDist->value());

}

void ConfigDialog::on_buttonBox_accepted()
{
    save_settings();
}
