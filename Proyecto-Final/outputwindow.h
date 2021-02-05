#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

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
    void setup(int cores, int deltas, QString clasificacion, int terminaciones, int bifurcaciones);

private:
    Ui::OutputWindow *ui;
};

#endif // OUTPUTWINDOW_H
