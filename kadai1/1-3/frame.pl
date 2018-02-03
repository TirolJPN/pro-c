#!/usr/bin/perl
use Tk;

$frame1 = $mw->Frame();
$frame1->Label(-text => "Your name? ")->pack(-side => ’left’);
$frame1->Entry(-textvariable => \$name)->pack(-side => ’right’);
$frame1->pack(-side => ’top’);
$frame2 = $mw->Frame();
$frame2->Button(-text => "Print", -command => \&hello)->pack(-side => ’left’);
$frame2->Button(-text => "Exit", -command => sub {exit})->pack(-side => ’right’);
$frame2->pack(-side => ’bottom’);
