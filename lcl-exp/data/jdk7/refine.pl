#!/usr/bin/perl

use strict;
my %calls;
my %returns;
my %fields;
my $openfile = "xalan.dot";

open (FH, $openfile) or die "sb";

while (my $line = <FH>){
    if($line =~ /\[calllabel=\"(.*?)\"/){
#	my $tmp = keys %calls;
	$calls{$1} = 1;
    }
    if($line =~ /\[retlabel=\"(.*?)\"/){
#	my $tmp = keys %returns;
	$returns{$1} = 1;
    }
    if($line =~ /\[label=\"(.*?)\"/){
#	my $tmp = keys %fields;
	$fields{$1} = 1;
    }






}
my $size = keys %calls;
my $size1 = keys %returns;
my $size2 = keys %fields;
print $openfile. " $size and $size1 and $size2\n";


my $i=0;
foreach my $key (keys %calls){
    if(exists $returns{$key}){
	$calls{$key}=$i; $i++; 
    }else{
	delete $calls{$key};
    }
}
$i=0;
foreach my $key (keys %returns){
    if(exists $calls{$key}){
	$returns{$key}=$i; $i++; 
    }else{
	delete $returns{$key};
    }

}
$i=0;
foreach my $key (keys %fields){$fields{$key}=$i; $i++; }


$size = keys %calls;
$size1 = keys %returns;
$size2 = keys %fields;
print "After refine call $size and return $size1 and field $size2\n";




close(FH);

open (FH, $openfile) or die "sb";
while (my $line = <FH>){
    if($line =~ /\[calllabel=\"(.*?)\"/){

	if(exists $returns{$1}){
	    my $i = $returns{$1};
	    $line =~ s/\[calllabel.*//;
	    chomp($line);
	    
	    print $line . "[label=\"op--$i\"]\n";
	}
    }
    if($line =~ /\[retlabel=\"(.*?)\"/){
	if(exists $calls{$1}){
	    my $i = $calls{$1};
	    $line =~ s/\[retlabel.*//;
	    chomp($line);
	    
	    print $line . "[label=\"cp--$i\"]\n";

	}
    }
    if($line =~ /\[label=\"(.*?)\"/){
	if(exists $fields{$1}){
	    my $i = $fields{$1};
	    $line =~ s/\[label.*//;
	    chomp($line);
	    
	    print $line . "[label=\"cb--$i\"]\n";

	    $line =~ s/\[label.*//;
	    $line =~ /(\d*)->(\d*)/;
	    my $from =$1;
	    my $to = $2;
	    print "$to->$from"."[label=\"ob--$i\"]\n";;





	}

    }

}
close(FH);







