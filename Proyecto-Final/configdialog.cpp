#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{

    ui->setupUi(this);
    ui->comboBox_enh->addItem("gabor");
    ui->comboBox_enh->addItem("none");

    ui->comboBox_thi->addItem("none");
    ui->comboBox_thi->addItem("zhangsuen");
    ui->comboBox_thi->addItem("guohall");
    ui->comboBox_thi->addItem("morph");

    ui->comboBox_mas->addItem("true");
    ui->comboBox_mas->addItem("false");

    ui->comboBox_kp->addItem("harris");
    ui->comboBox_kp->addItem("shitomasi");
    ui->comboBox_kp->addItem("ksurf");

    ui->comboBox_feat->addItem("orb");
    ui->comboBox_feat->addItem("dsurf");
    ui->comboBox_feat->addItem("dsift");

    ui->checkBox_draw->setChecked(true);

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

    ui->comboBox_enh->setCurrentText(settings.value("enhancement_method").toString());
    ui->comboBox_thi->setCurrentText(settings.value("thinning_method").toString());
    ui->comboBox_mas->setCurrentText(settings.value("masking").toString());

    ui->comboBox_kp->setCurrentText(settings.value("keypoint_extractor").toString());
    ui->comboBox_feat->setCurrentText(settings.value("feature_extractor").toString());

    ui->lineEdit_thresh->setText(settings.value("keypoint_threshold").toString());
    ui->lineEdit_dist->setText(settings.value("max_match_dist").toString());

    ui->checkBox_draw->setChecked(settings.value("draw_features").toBool());
}

void ConfigDialog::save_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    settings.setValue("enhancement_method", ui->comboBox_enh->currentText());
    settings.setValue("thinning_method", ui->comboBox_thi->currentText());
    settings.setValue("masking", ui->comboBox_mas->currentText());

    settings.setValue("keypoint_extractor", ui->comboBox_kp->currentText());
    settings.setValue("feature_extractor", ui->comboBox_feat->currentText());

    settings.setValue("keypoint_threshold", ui->lineEdit_thresh->text());
    settings.setValue("max_match_dist", ui->lineEdit_dist->text());

    settings.setValue("draw_features",ui->checkBox_draw->isChecked());
}

void ConfigDialog::on_buttonBox_accepted()
{
    save_settings();
}
