#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "common.h"

#include <QDialog>
#include <QSettings>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void on_buttonBox_accepted();

    void on_btn_default_clicked();

    void on_doubleSpinBox_maxEdge_valueChanged(double arg1);

    void on_doubleSpinBox_minEdge_valueChanged(double arg1);

private:
    Ui::ConfigDialog *ui;

    void load_settings(bool def = false);
    void save_settings();
};

#endif // CONFIGDIALOG_H
