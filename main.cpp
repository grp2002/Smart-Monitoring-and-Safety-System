#include "window.h"

#include <QApplication>

// Main program
int main(int argc, char *argv[])
{
	/**
	 * QApplication initializes the application with the user's desktop settings and keeps track of these 
	 * properties in case the user changes the desktop globally. It performs event handling, meaning that it 
	 * receives events from the underlying window system and dispatches them to the relevant widgets. It 
	 * parses common command line arguments and sets its internal state accordingly.
	 */
	QApplication app(argc, argv);

	// create the window
	Window window;
	window.show();

	// execute the application
	return app.exec();
}
