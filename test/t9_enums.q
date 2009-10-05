#!/usr/bin/env qore
%requires qt4


sub compare($l, $r) {
    printf("%n == %n: %n\n", $l, $r, $l == $r);
}

sub ncompare($l, $r) {
    printf("%n != %n: %n\n", $l, $r, $l != $r);
}

sub hcompare($l, $r) {
    printf("%n === %n: %n\n", $l, $r, $l === $r);
}

sub hncompare($l, $r) {
    printf("%n !== %n: %n\n", $l, $r, $l !== $r);
}


printf("QAction::NoRole: %N\n", QAction::NoRole);
printf("QAction::TextHeuristicRole: %N\n", QAction::TextHeuristicRole);
printf("QAction::ApplicationSpecificRole: %N\n", QAction::ApplicationSpecificRole);
printf("QAction::AboutQtRole: %N\n", QAction::AboutQtRole);
printf("QAction::AboutRole: %N\n", QAction::AboutRole);
printf("QAction::PreferencesRole: %N\n", QAction::PreferencesRole);
printf("QAction::QuitRole: %N\n", QAction::QuitRole);

my $a = new QApplication();
my $w = new QAction("foobar", $a);
my $e = $w.menuRole();
printf("\ntmenuRole: %N\n", $e);


compare(QAction::NoRole, $e);
compare(QAction::TextHeuristicRole, $e);
compare(QAction::ApplicationSpecificRole, $e);
compare(QAction::AboutQtRole, $e);
compare(QAction::AboutRole, $e);
compare(QAction::PreferencesRole, $e);
compare(QAction::QuitRole, $e);
compare($e, "foo");
compare($e, 1);
printf("\n");
ncompare(QAction::NoRole, $e);
ncompare(QAction::TextHeuristicRole, $e);
ncompare(QAction::ApplicationSpecificRole, $e);
ncompare(QAction::AboutQtRole, $e);
ncompare(QAction::AboutRole, $e);
ncompare(QAction::PreferencesRole, $e);
ncompare(QAction::QuitRole, $e);
ncompare($e, "foo");
ncompare($e, 1);
ncompare($e, 0);
printf("\n");
hcompare(QAction::NoRole, $e);
hcompare(QAction::TextHeuristicRole, $e);
hcompare(QAction::ApplicationSpecificRole, $e);
hcompare(QAction::AboutQtRole, $e);
hcompare(QAction::AboutRole, $e);
hcompare(QAction::PreferencesRole, $e);
hcompare(QAction::QuitRole, $e);
hcompare($e, "foo");
hcompare($e, 1);
hcompare($e, 0);
printf("\n");
hncompare(QAction::NoRole, $e);
hncompare(QAction::TextHeuristicRole, $e);
hncompare(QAction::ApplicationSpecificRole, $e);
hncompare(QAction::AboutQtRole, $e);
hncompare(QAction::AboutRole, $e);
hncompare(QAction::PreferencesRole, $e);
hncompare(QAction::QuitRole, $e);
hncompare($e, "foo");
hncompare($e, 1);
hncompare($e, 0);


