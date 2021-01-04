#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{

    ui->setupUi(this);
    qDebug() << QApplication::applicationDirPath();
    ui->comboBox_enh->addItem("Gabor");
    ui->comboBox_enh->addItem("None");

    ui->comboBox_thi->addItem("ZhangSuen");
    ui->comboBox_thi->addItem("GuoHall");
    ui->comboBox_thi->addItem("Morph");

    ui->comboBox_mas->addItem("True");
    ui->comboBox_mas->addItem("False");

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

    app_settings.enhancement_method = (fp::Preprocesser::EnhancementMethod)settings.value("enhance_method").toInt()
    ui->comboBox_enh->setCurrentText(settings.value("enhance_method").toString());
    ui->comboBox_thi->setCurrentText(settings.value("thinning_method").toString());
    ui->comboBox_mas->setCurrentText(settings.value("masking").toString());
}

void ConfigDialog::on_buttonBox_accepted()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);
    settings.setValue("enhance_method", ui->comboBox_enh->currentText());
    settings.setValue("thinning_method", ui->comboBox_thi->currentText());
    settings.setValue("masking", ui->comboBox_mas->currentText());
}
