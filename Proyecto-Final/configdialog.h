#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "common.h"

#include <QDialog>
#include <QSettings>
#include <QDebug>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

    struct AppSettings
    {
        fp::Preprocesser::EnhancementMethod enhancement_method;
        fp::Preprocesser::ThinningMethod thinning_method;
        bool masking;
    };

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *ui;

    void load_settings();
    void save_settings();
    AppSettings app_settings;
};

#endif // CONFIGDIALOG_H
