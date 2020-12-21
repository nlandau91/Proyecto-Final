#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

void MainWindow::setup_db()
{
    //database setup
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db = QSqlDatabase::addDatabase(DRIVER);
        //db.setDatabaseName(":memory:");
        db.setDatabaseName("../db/fingerprint_db");
        if(!db.open())
        {
            qWarning() << "ERROR: " << db.lastError();
        }
        //creamos una tabla
        QSqlQuery query("CREATE TABLE people (id INTEGER, descriptor_sample INTEGER, descriptor_path TEXT,PRIMARY KEY(id, descriptor_sample))");
        if(!query.isActive())
        {
            //no se pudo crear la table, posiblemente porque ya existia
            //qWarning() << "ERROR: " << query.lastError().text();
        }
    }
}

//guarda el descriptor en disco y lo ingresa a la base de datos
void MainWindow::ingresar_descriptor(cv::Mat &descriptors, const QString &id)
{
    //armamos el path
    QDir qdir = QDir::current();
    QString descriptor_path = qdir.path()+"/../db/descriptors/"+id;
    qdir.mkpath(descriptor_path);

    //vemos que numero de muestra estamos por ingresar
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM people WHERE id=:id");
    query.bindValue(":id",id);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
    }
    int sample = 0;
    if(query.first())
    {
        sample = query.value(0).toInt();
    }
    //guardamos el archivo en disco armando el nombre
    descriptor_path = descriptor_path + "/" + QString::number(sample)+".jpg";
    cv::imwrite(descriptor_path.toStdString(), descriptors);
    //ingresamos los descriptores a la base de datos
    query.prepare("INSERT INTO people(id, descriptor_sample, descriptor_path) VALUES (:id, :sample, :path)");
    query.bindValue(":id",id);
    query.bindValue(":path", descriptor_path);
    query.bindValue(":sample",sample);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
    }
}

//devuelve los descriptores de las muestras correspondientes al id
std::vector<cv::Mat> MainWindow::obtener_lista_descriptores(const QString &id)
{
    std::vector<cv::Mat> lista_descriptores;

    QSqlQuery query;
    query.prepare("SELECT descriptor_path FROM people WHERE id=:id");
    query.bindValue(":id",id);
    if(!query.exec())
    {
        qWarning() << "ERROR: " << query.lastError().text();
    }
    while(query.next())
    {
        QString path = query.value(0).toString();
        std::cout << path.toStdString() << std::endl;
        cv::Mat descriptores = cv::imread(path.toStdString(),cv::IMREAD_GRAYSCALE);
        lista_descriptores.push_back(descriptores);
    }


    //devolvemos los descriptores
    return lista_descriptores;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_db();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//convierte una imagen de tipo cv::Mat a una de tipo QImage
inline QImage cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
        static QVector<QRgb>  sColorTable;

        // only create our color table once

        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );
        return image;
    }

    default:
    {
        break;
    }

    }

    return QImage();
}

//convierte una imagen de tipo cv::Mat a una de tipo QPixmap
inline QPixmap cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}

void MainWindow::mostrarImagen(cv::Mat &imagen)
{
    ui->label->setPixmap(cvMatToQPixmap(imagen));
}

//realiza una iteracion de reduccion, la misma se debe repetir hasta que la imagen este esquelitizada
void thinningIteration(cv::Mat& im, int iter)
{
    cv::Mat marker =cv::Mat::zeros(im.size(), CV_8UC1);
    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                    (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                    (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                    (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

//funcion para reducir una imagen binaria, debe estar en el rango de 0-255.
//basado en el algoritmo de Zhang-Suen https://rosettacode.org/wiki/Zhang-Suen_thinning_algorithm
void thinning(cv::Mat& im)
{
    // Enforce the range tob e in between 0 - 255
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}

//preprocesamos la imagen
cv::Mat MainWindow::preprocesar(cv::Mat &src)
{
    //pasamos la imagen de escala de gris a binario
    cv::Mat binary;
    cv::threshold(src,binary,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    //luego obtenemos el "esqueleto" de la imagen
    cv::Mat skeleton = binary.clone();
    thinning(skeleton);
    return skeleton;
}

//buscamos los keypoints en una imagen con el detector de esquinas de Harris
//la imagen ya debe estar preprocesada
std::vector<cv::KeyPoint> obtenerKeyPoints(cv::Mat preprocesado, float threshold = 125.0)
{
    cv::Mat harris_corners, harris_normalised;
    harris_corners = cv::Mat::zeros(preprocesado.size(), CV_32FC1);
    cv::cornerHarris(preprocesado,harris_corners,2,3,0.04,cv::BORDER_DEFAULT);
    cv::normalize(harris_corners,harris_normalised,0,255,cv::NORM_MINMAX,CV_32FC1,cv::Mat());
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat rescaled;
    cv::convertScaleAbs(harris_normalised,rescaled);
    cv::Mat harris_c(rescaled.rows,rescaled.cols,CV_8UC3);
    cv::Mat in[] = {rescaled,rescaled,rescaled};
    int from_to[] = {0,0,1,1,2,2};
    cv::mixChannels(in,3,&harris_c,1,from_to,3);
    for(int x = 0; x < harris_normalised.cols; x++)
    {
        for(int y = 0; y < harris_normalised.rows; y++)
        {
            if((int)harris_normalised.at<float>(y,x) > threshold)
            {
                //guardamos el keypoint
                keypoints.push_back((cv::KeyPoint(x,y,1)));
            }
        }
    }
    return keypoints;
}

//a partir de una imagen en escala de gris, obtiene sus descriptores
//pensado para huellas digitales
cv::Mat MainWindow::calcular_descriptores(cv::Mat &src)
{
    //preprocesamos la imagen para mejorar la extraccion de caracteristicas
    cv::Mat preprocesado = preprocesar(src);
    //buscamos los puntos minuciosos (minutae)
    std::vector<cv::KeyPoint> keypoints;
    keypoints = obtenerKeyPoints(preprocesado);
    //obtenemos los descriptores alrededor de esos puntos
    cv::Ptr<cv::Feature2D> orb_descriptor = cv::ORB::create();
    cv::Mat descriptors;
    orb_descriptor->compute(preprocesado,keypoints,descriptors);
    return descriptors;
}

void MainWindow::on_btn_ingresar_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Image"), "../res/",
                                                          tr("Images (*.jpg *.jpeg *.jpe *.jp2 *.png *.bmp *.dib *.tif);;All Files (*)"));
    std::string id = ui->lineEdit->text().toStdString();
    for(QString fileName : fileNames)
    {
        //leemos la imagen en escala de gris
        cv::Mat src = cv::imread(fileName.toStdString(),cv::IMREAD_GRAYSCALE);
        if(!src.empty())
        {
            //obtenemos los descriptores
            cv::Mat descriptors = calcular_descriptores(src);
            //guardamos el descriptor e ingresamos los descriptores a la base de datos
            QString id = ui->lineEdit->text();
            ingresar_descriptor(descriptors,id);
            std::cout << "Huella ingresada" << std:: endl;
        }
    }
}

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
std::vector<std::vector<cv::DMatch>> obtenerMatches(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(cv::NORM_HAMMING);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > all_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<cv::DMatch> matches;
        matcher.match(lista_descriptores[entry],descriptors,matches);
        all_matches.push_back(matches);
        // Loop over matches and multiply
        // Return the matching certainty score
        if(matches.size() > 0)
        {
            float score = 0.0;
            float sum = 0.0;
            for(long unsigned i=0; i < matches.size(); i++){
                cv::DMatch current_match = matches[i];
                sum = sum + current_match.distance;
            }
            score = sum/matches.size();
            std::cerr << std::endl << "Current matching score = " << score << std::endl;
        }
    }
    return all_matches;
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
            //obtenemos los descriptores
            cv::Mat descriptors = calcular_descriptores(src);
            //obtenemos la lista de descriptores de la base de datos
            QString id = ui->lineEdit->text();
            std::vector<cv::Mat> lista_descriptores;
            lista_descriptores = obtener_lista_descriptores(id);
            //obtenemos los matches entre los descriptores de la imagen ingresada, y los de la base de datos
            std::vector<std::vector<cv::DMatch>> matches = obtenerMatches(descriptors, lista_descriptores);
            //analizamos los matches para intentar verificar

        }
    }
}
