#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "database.h"
#include "appsettings.h"
#include "preprocesser.h"
#include "analyzer.h"
#include "comparator.h"
#include "utils.h"
#include "outputwindow.h"
#include "fingerprinttemplate.h"
#include "tester.h"

#include <QMainWindow>
#include <QFileDialog>

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

    void on_btn_demo_clicked();

private:
    Ui::MainWindow *ui;
    OutputWindow *output_window;
    fp::Database db;
    fp::AppSettings app_settings;
    fp::Preprocesser preprocesser;
    fp::Analyzer analyzer;
    fp::Comparator comparator;
    void load_settings();

};
#endif // MAINWINDOW_H
