#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::string src_path = "test.jpg";
    cv::Mat src = cv::imread(src_path,cv::IMREAD_GRAYSCALE);
}

MainWindow::~MainWindow()
{
    delete ui;
}

