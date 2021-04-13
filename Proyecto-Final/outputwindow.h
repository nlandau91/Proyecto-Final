#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include "common.h"
#include "utils.h"
#include "fingerprinttemplate.h"

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
    void setup(const fp::FingerprintTemplate &fp_template, const fp::Preprocessed &prep);

private slots:
    void on_radioButton_filtered_clicked();

    void on_radioButton_thinned_clicked();

    void on_checkBox_marks_stateChanged(int arg1);

    void on_radioButton_orientation_clicked();

    void on_radioButton_normalized_clicked();

    void on_radioButton_original_clicked();

    void on_checkBox_segment_stateChanged(int arg1);

private:
    Ui::OutputWindow *ui;
    fp::FingerprintTemplate fp_template;
    fp::Preprocessed prep;
    cv::Mat selected;
    bool drawover;
    bool segment;
    void set_output();
};

#endif // OUTPUTWINDOW_H
