#!/usr/bin/perl
#
# Very Simple Editor
#
use strict;
use warnings;
use Tk;

my $mw = MainWindow->new();
$mw->title("Very Simple Editor");

#
# ボタンとファイル名エントリを格納するフレームを作成
#
my $top = $mw->Frame(-borderwidth => 10);
$top->pack(-side => 'top', -fill => 'x');

my $top2 = $mw->Frame(-borderwidth => 10);
$top2->pack(-side => 'top', -fill => 'x');

#
# ボタンとファイル名エントリを作成, 配置
#
my $quit = $top->Button(-text => 'Quit', -command => sub { exit });
my $clear = $top->Button(-text => 'Clear', -command => \&clear_text);
my $save = $top->Button(-text => 'Save', -command => \&save_text);
my $label = $top->Label(-text => 'Filename:', -padx => 0);
my $savefilename;
my $filename = $top->Entry(-width => 30,
						   -relief => 'sunken',
						   -textvariable => \$savefilename);
$label->pack(-side => 'left');
$filename->pack(-side => 'left');
$quit->pack(-side => 'right');
$clear->pack(-side => 'right');
$save->pack(-side => 'right');

my $load = $top2->Button(-text => 'Load', -command => \&load_text);
my $insert = $top2->Button(-text => 'Insert', -command => \&insert_text);
my $loadfilename;
my $filename2 = $top2->Entry(-width => 20,
						   -relief => 'sunken',
						   -textvariable => \$loadfilename);
my $label2 = $top2->Label(-text => 'Load / Insert filename:', -padx => 0);
$label2->pack(-side => 'left');
$filename2->pack(-side => 'left');
$load->pack(-side => 'right');
$insert->pack(-side => 'right');
# エディットに利用するテキストウィジェットを作成
#
my $textframe = $mw->Frame();
my $scroll = $textframe->Scrollbar();
my $textfield =
	$textframe->Text(-width => 80,
					 -height => 25,
					 -borderwidth => 2,
					 -relief => 'sunken',
					 -setgrid => 1,
					 -yscrollcommand => ['set' => $scroll]);
$scroll->configure(-command => ['yview' => $textfield]);
$scroll->pack(-side => 'right', -fill => 'y');
$textfield->pack(-side => 'left', -fill => 'both', -expand => 1);
$textframe->pack(-side => 'top', -fill => 'both', -expand => 1);

#
# 表示
#
MainLoop;

#
# Clearボタンに対応する手続き
# エディット中のテキストをすべて削除する
#
sub clear_text {
    $textfield->delete('1.0','end');
}

#
# Saveボタンに対応する手続き
# filenameエントリに記述されたファイル名でファイルをオープンし,
# テキストウィジェットの内容をそのファイルへ書き込む
#
sub save_text {
    open(OUTFILE, ">$savefilename")
	or warn("Can't open '$savefilename'"), return;
    print OUTFILE $textfield->get('1.0','end');
    close(OUTFILE);
}

sub load_text {
		if($loadfilename){
			$textfield->delete('1.0','end');
			open(LOADFILE, "<$loadfilename")
			or warn("Can't open '$loadfilename'"), return;
			while(<LOADFILE>){
				$textfield->insert('end', $_)
			}
			close(LOADFILE);
		}
}

sub insert_text {
		if($loadfilename){
			open(LOADFILE, "<$loadfilename")
			or warn("Can't open '$loadfilename'"), return;
			while(<LOADFILE>){
				$textfield->insert('end', $_)
			}
			close(LOADFILE);
		}
}
