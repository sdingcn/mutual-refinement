#!/usr/bin/perl


use strict;

open (FH, "refined/antlr.dot");

foreach my $line(<FH>){

    if($line =~ /op--/){
	$line =~ s/op--/ob--/g;
	print $line;
    }elsif($line =~ /ob--/){
	$line =~ s/ob--/op--/g;
	print $line;
    }elsif($line =~ /cp--/){
	$line =~ s/cp--/cb--/g;
	print $line;
    }elsif($line =~ /cb--/){
	$line =~ s/cb--/cp--/g;
	print $line;
    }else{
	print $line;
    }
}



close(FH);
