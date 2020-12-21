#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <opencv2/opencv.hpp>

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

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

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;

    void ingresar_descriptor(cv::Mat &descriptors, const QString &id);
    std::vector<cv::Mat> obtener_lista_descriptores(const QString &id);
    void setup_db();
    void mostrarImagen(cv::Mat &imagen);
    cv::Mat calcular_descriptores(cv::Mat &src);
    cv::Mat preprocesar(cv::Mat &src);

};
#endif // MAINWINDOW_H
