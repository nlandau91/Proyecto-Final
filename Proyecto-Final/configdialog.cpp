#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{

    ui->setupUi(this);
    qDebug() << QApplication::applicationDirPath();
    ui->comboBox_enh->addItem("gabor");
    ui->comboBox_enh->addItem("none");

    ui->comboBox_thi->addItem("zhangsuen");
    ui->comboBox_thi->addItem("guoHall");
    ui->comboBox_thi->addItem("morph");

    ui->comboBox_mas->addItem("true");
    ui->comboBox_mas->addItem("false");

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

    ui->comboBox_enh->setCurrentText(settings.value("enhance_method").toString());
    ui->comboBox_thi->setCurrentText(settings.value("thinning_method").toString());
    ui->comboBox_mas->setCurrentText(settings.value("masking").toString());
}

void ConfigDialog::save_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    settings.setValue("enhance_method", ui->comboBox_enh->currentText());
    settings.setValue("thinning_method", ui->comboBox_thi->currentText());
    settings.setValue("masking", ui->comboBox_mas->currentText());
}

void ConfigDialog::on_buttonBox_accepted()
{
    save_settings();
}
