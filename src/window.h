#ifndef WINDOW_H
#define WINDOW_H

/*
 * Special thanks to Bernd Porr <mail@berndporr.me.uk> for contributing
 * and sharing his work as free software in public domain. This software
 * is re-using parts of his work.
 * 
 */

#include <qwt/qwt_thermo.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <QBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QDateTime>
#include <QHeaderView>

#include <mutex>

// class definition 'Window'. It inherits QWidget which inherits QObject.
class Window : public QWidget
{
    // must include the Q_OBJECT macro for for the Qt signals/slots framework to work with this class
    Q_OBJECT
    
public:
    Window(); // default constructor - called when a Window is declared without arguments
    ~Window();
 
// mark the method as a slot to make it Q_INVOKABLE function    
public slots:
    void updateTemperature(double temp);


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

    std::mutex mtx; 
};

#endif // WINDOW_H
