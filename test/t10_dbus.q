#!/usr/bin/env qore
%requires qt4

my $conn = QDBusConnection::sessionBus();
printf("sessionBus: %N\n", $conn);

printf("isConnected: %N\n", $conn.isConnected());
if (!$conn.isConnected()) {
	printf("err: %N - %N\n", $conn.lastError().name(), $conn.lastError().message());
	exit(1);
}

my $iface = $conn.interface();
printf("interface: %N\n", $iface);


printf("list: %N\n", $iface.registeredServiceNames());

