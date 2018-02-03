#!/usr/bin/perl
use Tk;
$mw = MainWindow->new();
$mw->title("Button Demo");
$button = $mw->Button(-text => "Exit", -command => sub {exit});
$button->pack();
MainLoop;
