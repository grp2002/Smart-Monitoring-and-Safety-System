#include "window.h"

Window::Window()
{    
    // set up the thermometer
    thermo = new QwtThermo; 
    thermo->setFillBrush( QBrush(Qt::red) );
    thermo->setScale(23, 25);
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
    
    plot->setAxisScale(QwtPlot::yLeft,23,25);
    plot->replot();
    plot->show();

    // add table to display the temperature values
    table = new QTableWidget;

    // set table structure
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Time", "Temperature (°C)"});
    table->setRowCount(1); // Only 1 row

    // set Font size
    QFont font; 
    font.setPointSize(14); 
    table->setFont(font);
    table->horizontalHeader()->setFont(font);

    // table formatting
    table->setStyleSheet("QTableWidget::item { padding: 6px; }");
    table->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // <== IMPORTANT
    table->verticalHeader()->setVisible(false);
 
    table->setMinimumWidth(320);

    // Widget styles
    thermo->setStyleSheet("QwtThermo { border: 1px solid gray; padding: 5px; }");
    plot->setStyleSheet("QwtPlot { border: 1px solid #444; padding: 8px; }");
    table->setStyleSheet(
        "QTableWidget { border: 1px solid #222; padding: 10px; }"
        "QTableWidget::item { padding: 8px; }"
    );

    // reset button
    button = new QPushButton("Reset");
    // see https://doc.qt.io/qt-5/signalsandslots-syntaxes.html
    connect(button,&QPushButton::clicked,[this](){reset();});

    // set up the layout - button above thermometer
    vLayout = new QVBoxLayout();
    vLayout->setSpacing(10);  // set spacing between button and thermo
    vLayout->setContentsMargins(5, 5, 5, 5);  // set margin around the vertical layout

    vLayout->addWidget(button);
    vLayout->addWidget(thermo);     

    // plot to the left of button and thermometer
    hLayout = new QHBoxLayout();
    hLayout->setSpacing(20);  // set spacing between thermo, plot and table
    hLayout->setContentsMargins(10, 10, 10, 10); // set margin

    hLayout->addLayout(vLayout);
    hLayout->addWidget(plot);
    hLayout->addWidget(table);

    hLayout->setStretch(0, 2);
    hLayout->setStretch(1, 4);
    hLayout->setStretch(2, 4);

    setLayout(hLayout);

    this->setMinimumSize(900, 300); 

}

Window::~Window() {  
}

void Window::reset() {
    std::lock_guard<std::mutex> lock(mtx);

    for (int index = 0; index < plotDataSize; ++index) {
        yData[index] = 0;
    }

    // Reset thermo and plot range to default
    thermo->setScale(23, 25);
    thermo->setValue(0);

    curve->setSamples(xData, yData, plotDataSize);
    plot->setAxisScale(QwtPlot::yLeft, 23, 25);
    plot->replot();

    // Reset table
    table->setItem(0, 0, new QTableWidgetItem(""));
    table->setItem(0, 1, new QTableWidgetItem(""));
}

/* keep the scale small to view the variations
void Window::updateTemperature(double temp) {
    std::lock_guard<std::mutex> lock(mtx);

    // Shift data for rolling plot
    std::move(yData, yData + plotDataSize - 1, yData + 1);
    yData[0] = temp;

    // Update table
    table->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    table->setItem(0, 1, new QTableWidgetItem(QString::number(temp, 'f', 2)));

    // Dynamically scale thermo
    double range = 0.1;
    thermo->setScale(temp - range, temp + range);
    thermo->setValue(temp);

    // Plotting
    plot->setAxisScale(QwtPlot::yLeft, temp - 0.1, temp + 0.1);
    curve->setSamples(xData, yData, plotDataSize);
    plot->replot();
    update();
}
*/
/* Refresh plot when temperature value goes out of range
void Window::updateTemperature(double temp) {
    std::lock_guard<std::mutex> lock(mtx);

    // Shift data for rolling plot
    std::move(yData, yData + plotDataSize - 1, yData + 1);
    yData[0] = temp;

    // Update table
    table->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    table->setItem(0, 1, new QTableWidgetItem(QString::number(temp, 'f', 2)));

    static double currentMin = 23.0;
    static double currentMax = 25.0;
    double margin = 0.2;

    // Adjust range only if temp is out of bounds
    if (temp < currentMin || temp > currentMax) {
        currentMin = temp - margin;
        currentMax = temp + margin;

        plot->setAxisScale(QwtPlot::yLeft, currentMin, currentMax);
        thermo->setScale(currentMin, currentMax);
    }

    // Update current value
    thermo->setValue(temp);
    curve->setSamples(xData, yData, plotDataSize);
    plot->replot();
    update();
}
*/
///* Auto scale as per visible data
void Window::updateTemperature(double temp) {
    std::lock_guard<std::mutex> lock(mtx);

    // Shift data for rolling plot
    std::move(yData, yData + plotDataSize - 1, yData + 1);
    yData[0] = temp;

    // Update table
    table->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    table->setItem(0, 1, new QTableWidgetItem(QString::number(temp, 'f', 2)));

    // Calculate new range from current data
    double min = yData[0];
    double max = yData[0];
    for (int i = 1; i < plotDataSize; ++i) {
        if (yData[i] < min) min = yData[i];
        if (yData[i] > max) max = yData[i];
    }

    // Add padding/margin to range
    double margin = 0.1;
    min -= margin;
    max += margin;

    // Update UI
    plot->setAxisScale(QwtPlot::yLeft, min, max);
    thermo->setScale(min, max);
    thermo->setValue(temp);
    curve->setSamples(xData, yData, plotDataSize);
    plot->replot();
    update();
}
//*/