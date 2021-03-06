#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include "preprocessed.h"
#include "utils.h"
#include "fingerprinttemplate.h"

#include <QWidget>
namespace Ui {
class OutputWindow;
}

/*!
 * La clase OutputWindow se encarga de mostrar en una ventana la informacion
 * de una huella dactilar preprocesada y analizada
 */

class OutputWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OutputWindow(QWidget *parent = nullptr);
    ~OutputWindow();
    /*!
     * \brief setup arma el panel de salida a partir del preprocesad y template de una huella dactilar
     * \param fp_template template de la huella dactilar
     * \param prep preprocesado de la huella dactilar
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

    void on_pushButton_guardar_clicked();

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
