#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <common.h>
#include <opencv2/opencv.hpp>

#include <QMainWindow>
#include <QFileDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    struct AppSettings
    {
        fp::Preprocesser::EnhancementMethod enhancement_method;
        fp::Preprocesser::ThinningMethod thinning_method;
        bool masking;
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_ingresar_clicked();

    void on_btn_verificar_clicked();

    void on_btn_identificar_clicked();

    void on_actionOpciones_triggered();

private:
    Ui::MainWindow *ui;
    fp::Database db;
    AppSettings app_settings;
    EnumParser<int> enum_parser;
    void mostrar_imagen(cv::Mat &imagen);
    void load_settings();

};
#endif // MAINWINDOW_H
