/**
 * Version Details: 
 * ABOUT: Smart Monitoring and Safety System software to monitor temperature sensor 
 * via blocking IO in an interrupt driven model and calling the event handler callback
 * functions for event notification.
 * INPUT: two TMP117 temperature sensors and a PIR motion sensor
 * OUTPUT: 
 * (1) Console output on the terminal
 * (2) Trigger the buzzer beep 
 * (3) Display the real time temperature values using QT Thermo, Plot and Table widgets
 */

 /*
 * Copyright (c) 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. See the file LICENSE.
 */

 #include <cstdlib> // for system()
 #include <stdint.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <linux/i2c-dev.h>
 #include <sys/ioctl.h>
 #include <poll.h>
 #include <error.h>
 #include <string.h>
 #include <cmath>
 #include "gpioevent.h"
 #include "buzzer.h"
 #include "SafePrint.h" //safe printf in multi-threaded environment
 #include "TMP117TemperatureSensor.h"
 #include "SensorMsgPublisher.h"
 #include "MotionSensor.h"
 #include <mutex>
 //For QT:
 #include <QApplication>
 #include <csignal>
 #include "window.h"
 
 // Mutex to protect the shared I2C bus in a multi-threaded environment
 static std::mutex i2c_mutex; 
 
 //For QT safe exit:
 void handleSignal(int signal) {
	 if (signal == SIGINT || signal == SIGTERM) {
		 qApp->quit();  // Safe Qt application exit
	 }
 }
 
 int main(int argc, char *argv[]) {
 
 //SafePrint::printf("****Press any key to stop.****\n\r");
 SafePrint::printf("**** Close all the QT windows to stop. ****\n\r");
 SafePrint::printf("**** If running from a remote SSH window run 'export DISPLAY=:0' ****\n\r");
 SafePrint::printf("**** Run 'reset' command after stop if terminal is non-responsive due to QT. ****\n\n\r");
 
 //Publisher Start
 SafePrint::printf("Starting message publisher.\n");
 
 SensorMsgPublisher msgPublisher;
 
 if(!msgPublisher.init())
 {
	 SafePrint::printf("Message Publisher not init'd...\n\r");
	 //return -1;
 }
 
 //Publisher End
 
 //QT Start
	 QApplication app(argc, argv);
	 
	 Window sensor1Window, sensor2Window;
	 sensor1Window.setWindowTitle("TMP117 Sensor 1");
	 sensor2Window.setWindowTitle("TMP117 Sensor 2");
 
	 // Position windows
	 sensor1Window.move(100, 100);     // Top window
	 sensor2Window.move(100, 500);     // Below the first window
 
	 sensor1Window.show();
	 sensor2Window.show();
 //QT End
	 
	 Buzzer shared_buzzer = Buzzer(0, 25); //initialize Buzzer at gpiochip0 on pin 25
	 
	 GPIOPin gpiopin17, gpiopin27, gpiopin23;
	 const int gpioPinNo17 = 17;
	 const int gpioPinNo27 = 27;
	 const int gpioPinNo23 = 23;
 
	 TMP117TemperatureSensor t1 = TMP117TemperatureSensor(1, &shared_buzzer);
	 /**
	  * Callback implementation using a lambda (inline) function. 
	  * It connects non-Qt backend logic (t1, the TMP117 sensor) to a Qt GUI method (updateTemperature).
	  * @QMetaObject::invokeMethod() ensures that the GUI update runs in the Qt main thread, which is 
	  * thread-safe and essential when callback runs from a separate thread (main()).
	  */
	 t1.onTemperatureRead = [&](double t) {
		 QMetaObject::invokeMethod(&sensor1Window, "updateTemperature", Qt::QueuedConnection,
								   Q_ARG(double, t));
	 };
	 t1.setSensorMsgPublisher(&msgPublisher);
 
	 TMP117TemperatureSensor t2 = TMP117TemperatureSensor(2, &shared_buzzer);
	 t2.onTemperatureRead = [&](double t) {
		 QMetaObject::invokeMethod(&sensor2Window, "updateTemperature", Qt::QueuedConnection,
								   Q_ARG(double, t));
	 };
	 t2.setSensorMsgPublisher(&msgPublisher);
 
	 t1.initialize();
	 gpiopin17.registerCallback(&t1);
	 gpiopin17.start(gpioPinNo17, 0, "TMP117 Temperature Sensor");
 
	 t2.initialize();
	 gpiopin27.registerCallback(&t2);
	 gpiopin27.start(gpioPinNo27, 0, "TMP117 Temperature Sensor");
	 
	 // Force a startup temperature read for both sensors
	 t1.readAndPrintStartupTemperature();
	 t2.readAndPrintStartupTemperature();
	 
	 //Motion Sensor
	 MotionSensor motionSensor(&shared_buzzer);
	 gpiopin23.registerCallback(&motionSensor);
	 gpiopin23.start(gpioPinNo23, 0, "PIR Motion Sensor");
	 
	 /**
	  * Qt applications must return app.exec() without blocking the main thread. 
	  * getchar() is blocking the Qt event loop; it's not needed with QT else it will lead to 
	  * Crashes or hangs. 
	  */
	 //getchar(); //Blocking for the main program :: exit on key press
 
	 // shutdown with QT
	 QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
		 gpiopin17.stop();
		 gpiopin27.stop();
	 });
 
	 //return 0;
	 return app.exec();
 }
 