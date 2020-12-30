#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

//inicializa la base de datos
void MainWindow::setup_db()
{
    //database setup
    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        db = QSqlDatabase::addDatabase(DRIVER);
        //db.setDatabaseName(":memory:");
        db.setDatabaseName("../db/fingerprint.db");
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
    if(!descriptors.empty())
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
        cv::Mat descriptores = cv::imread(path.toStdString(),cv::IMREAD_GRAYSCALE);
        lista_descriptores.push_back(descriptores);
    }


    //devolvemos los descriptores
    return lista_descriptores;
}

//devuelve una lista con los id dentro de la base de datos
std::vector<QString> MainWindow::obtener_lista_id()
{
    std::vector<QString> lista_id;
    QSqlQuery query;
    query.prepare("SELECT DISTINCT id FROM people");
    if(!query.exec())
    {

    }
    while(query.next())
    {
        QString id = query.value(0).toString();
        lista_id.push_back(id);
    }
    return lista_id;
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

void MainWindow::mostrar_imagen(cv::Mat &imagen)
{
    ui->label->setPixmap(cvMatToQPixmap(imagen));
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
            cv::Mat enhanced = FingerprintEnhancer::enhance(src,FingerprintEnhancer::SKELETONIZE);
            //obtenemos los descriptores
            FingerprintAnalyzer::FingerprintAnalysis analysis = FingerprintAnalyzer::analize(enhanced);
            //cv::Mat descriptors = FingerprintAnalyzer::calcular_descriptores(enhanced);
            //solo ingresamos huellas que sean suficientemente buenas
            if(analysis.descriptors.rows > 4)
            {
                //guardamos el descriptor e ingresamos los descriptores a la base de datos
                QString id = ui->lineEdit->text();
                ingresar_descriptor(analysis.descriptors,id);
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

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
std::vector<std::vector<cv::DMatch>> obtener_matches(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(cv::NORM_HAMMING, true);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > all_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<cv::DMatch> matches;
        matcher.match(lista_descriptores[entry],descriptors,matches);
        all_matches.push_back(matches);
    }
    return all_matches;
}

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
//version alternativa, devuelve mejores matches utilizando el metodo de Lowe para comparar
std::vector<std::vector<cv::DMatch>> obtener_matches2(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores, float ratio = 0.8)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(cv::NORM_HAMMING);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > all_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<std::vector<cv::DMatch>> matches;
        matcher.knnMatch(descriptors,lista_descriptores[entry],matches,2);
        std::vector<cv::DMatch> good_matches;
        for (long unsigned i = 0; i < matches.size(); ++i)
        {
            if (matches[i][0].distance < ratio * matches[i][1].distance)
            {
                good_matches.push_back(matches[i][0]);
            }
        }
        all_matches.push_back(good_matches);
    }
    return all_matches;
}

//devuelve el mejor score(el mas bajo) entre matches de los descriptores des1 con los de lista_descriptores
//devuelve true si se encuentra un match con un escore menor a max_score
bool verificar(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores, int max_score = 80)
{
    int best_score = 999;
    std::vector<std::vector<cv::DMatch>> all_matches;
    all_matches = obtener_matches(descriptors, lista_descriptores);
    for(std::vector<cv::DMatch> vm : all_matches)
    {
        if(vm.size() > 0)
        {
            float score = 0.0;
            float sum = 0.0;
            for(cv::DMatch m : vm)
            {
                sum = sum + m.distance;
            }
            score = sum/vm.size();
            if(score < best_score)
            {
                best_score = score;
            }
            //std::cout << score << std::endl;
        }
    }
    return best_score <= max_score;
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
            cv::Mat enhanced = FingerprintEnhancer::enhance(src,FingerprintEnhancer::SKELETONIZE);
            //obtenemos los descriptores
            FingerprintAnalyzer::FingerprintAnalysis analysis = FingerprintAnalyzer::analize(enhanced);
            //cv::Mat descriptors = FingerprintAnalyzer::calcular_descriptores(enhanced);
            //solo verificamos si la huella es buena
            bool verificado = false;
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit->text();
                std::vector<cv::Mat> lista_descriptores;
                lista_descriptores = obtener_lista_descriptores(id);
                ///verificamos
                verificado = verificar(analysis.descriptors, lista_descriptores,80);
            }
            std::cout << "Verificado: " << verificado << std::endl;
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
            cv::Mat enhanced = FingerprintEnhancer::enhance(src,FingerprintEnhancer::SKELETONIZE);
            //obtenemos los descriptores
            FingerprintAnalyzer::FingerprintAnalysis analysis = FingerprintAnalyzer::analize(enhanced);
            //cv::Mat descriptors = FingerprintAnalyzer::calcular_descriptores(enhanced);
            //solo verificamos si la huella es buena
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos una lista con los id de la base de datos
                std::vector<QString> lista_id;
                lista_id = obtener_lista_id();
                //para cada id, realizamos la verificacion
                bool verificado = false;
                QString best_id;
                for(QString id : lista_id)
                {
                    //obtenemos la lista de descriptores de la base de datos
                    std::vector<cv::Mat> lista_descriptores;
                    lista_descriptores = obtener_lista_descriptores(id);
                    //obtenemos el mejor resultado entre los match de los descriptores
                    verificado = verificar(analysis.descriptors, lista_descriptores);
                    if(verificado)
                    {
                        std::cout << "Match encontrado: " << id.toStdString() << std::endl;
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
