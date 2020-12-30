#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = fp::Database();
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
            //cv::Mat descriptors = FingerprintAnalyzer::calcular_descriptores(enhanced);
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
            cv::Mat enhanced = fp::Enhancer::enhance(src,fp::Enhancer::SKELETONIZE);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
            //solo verificamos si la huella es buena
            bool verificado = false;
            if(analysis.descriptors.rows > 0)
            {
                //obtenemos la lista de descriptores de la base de datos
                QString id = ui->lineEdit->text();
                std::vector<cv::Mat> lista_descriptores;
                lista_descriptores = db.obtener_lista_descriptores(id);
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
            cv::Mat enhanced = fp::Enhancer::enhance(src,fp::Enhancer::SKELETONIZE);
            //obtenemos los descriptores
            fp::Analyzer::Analysis analysis = fp::Analyzer::analize(enhanced);
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
