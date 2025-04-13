#ifndef WINDOW_H
#define WINDOW_H

#include <qwt/qwt_thermo.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

#include <QBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QDateTime>

#include <mutex>

#include "fakesensor.h"

// class definition 'Window'
class Window : public QWidget, FakeSensor
{
    // must include the Q_OBJECT macro for for the Qt signals/slots framework to work with this class
    Q_OBJECT
    
public:
    Window(); // default constructor - called when a Window is declared without arguments
    ~Window();

    void timerEvent( QTimerEvent * );

// internal variables for the window class
private:
    static constexpr int plotDataSize = 100;

    QPushButton  *button;
    QwtThermo    *thermo;
    QwtPlot      *plot;
    QwtPlotCurve *curve;
    QTableWidget *table;

    

    // layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
    QVBoxLayout  *vLayout;  // vertical layout
    QHBoxLayout  *hLayout;  // horizontal layout

    // data arrays for the plot
    double xData[plotDataSize];
    double yData[plotDataSize];

    long count = 0;

    void reset();
    virtual void fakeSensorHasData(double v);

    std::mutex mtx; 
};

#endif // WINDOW_H
