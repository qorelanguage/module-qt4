#!/usr/bin/env qore
%requires qt4

%exec-class MyApp


class MyApp inherits QApplication
{
	constructor()
	{
		QObject::connect($self, SIGNAL("fontDatabaseChanged()"), $self, SLOT("fontChanged()"));
		$.connect($self, SIGNAL("aboutToQuit()"), SLOT("myQuit()"));
		$.connect($self, SIGNAL("destroyed()"), SLOT("myDestroyed()"));
		$.emit("fontDatabaseChanged()");
		QApplication::quit();
	}

	fontChanged()
	{
		printf("fontChanged\n");
	}

	myQuit()
	{
		printf("quit\n");
	}

	myDestroyed()
	{
		printf("destroyed\n");
	}
}

