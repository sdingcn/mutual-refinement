#!/usr/bin/perl


open(FH, "refined/chart.dot") or die;
my $no=1;
my $p=0;
foreach my $line(<FH>){
    

    if($no % 2){
	if(rand()<0.5){
	    print $line;
	}else{
	    $p=1;
	}
    }else{
	if($p){
	    print $line;
	}
	$p=0;
    }
    $no++;
}



close(FH);
