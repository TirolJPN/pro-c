#!/usr/bin/perl
#report1table.pl
foreach $arg (@ARGV){
	if(!($arg =~ /A|B|C|D|E|F|G|H/)){
		printf "Invalid argument.\n";
	}elsif(length($arg)!=1){
		printf "Invalid argument.\n";
	}
}
%result = ('-'=>'','O'=>'○','X'=>'×','='=>'△');
$file_num = @ARGV;

if ($file_num ==0){
	@file = (A,B,C,D,E,F,G,H);
}elsif($file_num <= 8){
	@file = @ARGV;
}else{
	printf "Invalid argument.\n";
	exit(1);
}
$tex_filename="group-";
foreach $arg (@file){
	$tex_filename .=$arg;
}
$tex_filename.=".tex";
open(TEX,">",$tex_filename);
binmode TEX,'encording(euc-jp)';
print TEX "\\documentclass[a4j]{jarticle}\n";
print TEX "\\begin{document}\n";
foreach $alpha (@file){
	$filepath = "report1-files/group".$alpha;
	open(FOO,$filepath);
	binmode FOO,'encording(euc-jp)';
	@all_lines = <FOO>;
	print TEX "\\begin{table}[htbp]\n";
	print TEX "\\begin{center}\n";
	print TEX "\\caption{\\underline{2014 FIFA World Cup Stage1 Result -Group ".$alpha."-}}\n";
	print TEX "\\begin{tabular}{|c|c|c|c|c|c|c|c|}\\hline\n";
	chomp($all_lines[3]);
	if ($all_lines[3] =~ /^\s+(.*)/) {
        	$all_lines[3] = $1;
    	}
	@team_name = split(/\s+/, $all_lines[3]);
	for($i=0; $i<4; $i++){
		$team_name[$i] =~ s/_/\\_/g;
	}
	print TEX "&".$team_name[0]."&".$team_name[1]."&".$team_name[2]."&".$team_name[3]."&勝ち試合数&引き分け試合数&負け試合数\\\\\\hline\n";
	$win = 0;
	$lose = 0;
	$draw = 0;
	@and1 = ('&','','','');
	@and2 = ('','&','','');
	@and3 = ('','','&','');
	@and4 = ('','','','&');
	for ($i=0; $i<4; $i++){
		$win++ while(@all_lines[$i+4] =~ m/O/g);
		$lose++ while(@all_lines[$i+4] =~ m/X/g);
		$draw = 3 - $win - $lose;
		chomp($all_lines[$i+4]);
		@team = split(/\s+/, $all_lines[$i+4]);
		$team[0] =~ s/_/\\_/g;
		print TEX $team[0]."&".$result{$team[1]}."&".$result{$team[2 + int(($i + 1)/ 3)]}."&".$result{$team[4 + int($i / 2)]}."&".$result{$team[6 + int($i / 3)]}."&".$win."&".$draw."&".$lose."\\\\\n";
		print TEX $and1[$i]."&".$team[2+int((3 - $i) / 3)].$and2[$i]."&".$team[4 + int((3 - $i) / 2)].$and3[$i]."&".$team[6 + int((5 - $i) / 3)].$and4[$i]."&&&\\\\\\hline\n";
		$win = 0;
		$lose = 0;
		$draw = 0;
	}
	print TEX "\\end{tabular}\n";
	print TEX "\\end{center}\n";
	print TEX "\\end{table}\n";
	close(FOO);
	print TEX "\\newpage\n";
}
print TEX "\\end{document}\n";
close(TEX);
