#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <common.h>
#include <opencv2/opencv.hpp>

#include <QMainWindow>
#include <QFileDialog>
#include "database.h"
#include "appsettings.h"
#include "preprocesser.h"
#include "analyzer.h"
#include "comparator.h"
#include "utils.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

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
    fp::Preprocesser preprocesser;
    fp::Analyzer analyzer;
    void mostrar_imagen(cv::Mat &imagen);
    void load_settings();

};
#endif // MAINWINDOW_H
