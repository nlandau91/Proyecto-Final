#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <opencv2/opencv.hpp>

#include <QMainWindow>

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

    void mostrarImagen(cv::Mat &imagen);

    std::vector<cv::Mat> database_descriptors;
};
#endif // MAINWINDOW_H
