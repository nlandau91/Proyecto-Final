#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H



#include <QDialog>
#include <QSettings>

namespace Ui {
class ConfigDialog;
}

/*!
 * La clase ConfigDialog es la que controla la ventana de configuracion del programa
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void on_buttonBox_accepted();

    void on_btn_default_clicked();

private:
    Ui::ConfigDialog *ui;

    void load_settings(bool def = false);
    void save_settings();
};

#endif // CONFIGDIALOG_H
