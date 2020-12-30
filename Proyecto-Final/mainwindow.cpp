#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db.init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mostrar_imagen(cv::Mat &imagen)
{
    ui->label->setPixmap(fp::cvMatToQPixmap(imagen));
}

void MainWindow::on_btn_ingresar_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Image"), "../res/",
                                                          tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    std::string id = ui->lineEdit->text().toStdString();
    int n = 0;
    for(QString fileName : fileNames)
    {
        ui->lineEdit->setText(QString::number(n/8));
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //mejoramos la imagen
            cv::Mat enhanced = fp::Enhancer::enhance(src,fp::Enhancer::SKELETONIZE);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            mostrar_imagen(analysis.fingerprint);
            //solo ingresamos huellas que sean suficientemente buenas
            if(analysis.descriptors.rows > 4)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                QString id = ui->lineEdit->text();
                db.ingresar_descriptor(analysis.descriptors,id);
                std::cout << "Huella ingresada" << std::endl;
            }
            else
            {
                std:: cout << "Huella no ingresada" << std::endl;
            }

        }
        n++;
    }
}

void MainWindow::on_btn_verificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //mejoramos la imagen
            cv::Mat enhanced = fp::Enhancer::enhance(src,fp::Enhancer::GABORFILTERS);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            mostrar_imagen(analysis.fingerprint);
            //solo verificamos si la huella es buena
            bool verificado = false;
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit->text();
                std::vector<cv::Mat> lista_descriptores;
                lista_descriptores = db.obtener_lista_descriptores(id);
                //verificamos
                verificado = 80>=fp::Comparator::compare(analysis.descriptors, lista_descriptores);
            }
            if(verificado)
            {
                std::cout << "Verificado!" << std::endl;
            }
            else
            {
                std::cout << "NO Verificado!" << std::endl;
            }
        }
    }
}

void MainWindow::on_btn_identificar_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "../res/",
                                                    tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    if(!fileName.isEmpty())
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //mejoramos la imagen
            cv::Mat enhanced = fp::Enhancer::enhance(src,fp::Enhancer::SKELETONIZE);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            mostrar_imagen(analysis.fingerprint);
            //solo verificamos si la huella es buena
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos una lista con los id de la base de datos
                std::vector<QString> lista_id;
                lista_id = db.obtener_lista_id();
                //para cada id, realizamos la verificacion
                bool verificado = false;
                QString best_id;
                for(QString id : lista_id)
                {
                    //obtenemos la lista de descriptores de la base de datos
                    std::vector<cv::Mat> lista_descriptores;
                    lista_descriptores = db.obtener_lista_descriptores(id);
                    //obtenemos el mejor resultado entre los match de los descriptores
                    verificado = 80>=fp::Comparator::compare(analysis.descriptors, lista_descriptores);
                    if(verificado)
                    {
                        std::cout << "Match encontrado: " << id.toStdString() << std::endl;;
                    }

                }
            }
            else
            {
                std::cout << "Ingrese la huella nuevamente" << std::endl;
            }
        }
    }
}
