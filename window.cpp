#include "window.h"

#include <cmath>  // for sine stuff


Window::Window()
{    
    // set up the thermometer
    thermo = new QwtThermo; 
    thermo->setFillBrush( QBrush(Qt::red) );
    thermo->setScale(-20, 50);
    thermo->show();
    
    
    // set up the initial plot data
    for( int index=0; index<plotDataSize; ++index )
    {
	xData[index] = index;
	yData[index] = 0;
    }
    
    curve = new QwtPlotCurve;
    plot = new QwtPlot;
    // make a plot curve from the data and attach it to the plot
    curve->setSamples(xData, yData, plotDataSize);
    curve->attach(plot);
    
    plot->setAxisScale(QwtPlot::yLeft,-10,10);
    plot->replot();
    plot->show();

    //Add table to display the temperature values
    table = new QTableWidget;
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Time", "Temperature (°C)"});
    table->setRowCount(1); // Only 1 row


    button = new QPushButton("Reset");
    // see https://doc.qt.io/qt-5/signalsandslots-syntaxes.html
    connect(button,&QPushButton::clicked,[this](){reset();});

    // set up the layout - button above thermometer
    vLayout = new QVBoxLayout();
    vLayout->addWidget(button);
    vLayout->addWidget(thermo);     

    // plot to the left of button and thermometer
    hLayout = new QHBoxLayout();
    hLayout->addLayout(vLayout);
    hLayout->addWidget(plot);
    hLayout->addWidget(table);

    setLayout(hLayout);

    // a fake data sample every 10ms
    FakeSensor::startms(10);
    // Screen refresh every 40ms
    startTimer(40);
}

Window::~Window() {
    FakeSensor::stop();
}

void Window::reset() {
    // set up the initial plot data
    for( int index=0; index<plotDataSize; ++index )
    {
	xData[index] = index;
	yData[index] = 0;
    }
}


// add the new input to the plot
void Window::fakeSensorHasData(double inVal) {
    mtx.lock();
    std::move( yData, yData + plotDataSize - 1, yData+1 );
    yData[0] = inVal;

    //Set table row values
    /**
    int row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(inVal, 'f', 2)));

        // Scroll to latest row
        table->scrollToBottom();
    */
   table->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
   table->setItem(0, 1, new QTableWidgetItem(QString::number(inVal, 'f', 2)));

    mtx.unlock();
}

// screen refresh
void Window::timerEvent( QTimerEvent * )
{
    mtx.lock();
    curve->setSamples(xData, yData, plotDataSize);
    thermo->setValue( fabs(yData[0]) );
    mtx.unlock();
    plot->replot();
    update();
}
