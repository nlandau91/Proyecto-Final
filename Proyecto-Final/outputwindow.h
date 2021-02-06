#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include "common.h"
#include "utils.h"

#include <QWidget>

namespace Ui {
class OutputWindow;
}

class OutputWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWindow(QWidget *parent = nullptr);
    ~OutputWindow();
    /*!
     * \brief setup arma el panel de salida a partir de los parametros pasados
     * \param analisis resultado del analisis de la huella
     * \param output imagen a mostrar en el panel
     */
    void setup(const fp::Analysis &analisis, const cv::Mat &output);

private:
    Ui::OutputWindow *ui;
};

#endif // OUTPUTWINDOW_H
