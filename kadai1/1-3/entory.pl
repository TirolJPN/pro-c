#!/usr/bin/perl
use Tk;
$mw = MainWindow->new();
$mw->title("Entry Demo");
$mw->Label(-text => "Your name? ")->pack(-side => ’left’);
$mw->Entry(-textvariable => \$name)->pack(-side => ’left’);
$mw->Button(-text => "Print", -command => \&hello)->pack(-side => ’left’);
$mw->Button(-text => "Exit", -command => sub {exit})->pack(-side => ’right’);
MainLoop;
sub hello {
  print "Hello, $name!\n";
}
