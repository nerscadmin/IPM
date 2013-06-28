#!/usr/bin/env perl

use Time::Local;
use Date::Format;

$jobid="";
$host="";
$start="";
$stop="";

%power  = ();
%energy = ();

while(<>) {
    chomp;

    if(/\<job.*start=\"(\d+)\".*final=\"(\d+)\".*\>(\S+)\<.*\>/ ) {
	$start=$1;
	$stop=$2;

	if( $jobid eq "" ) {
	    $jobid=$3;

	    read_powerdata(\%power, $jobid);
	    power_to_energy(\%power, \%energy);
	}
    }
    if(/\<host.*\>(\S+)\<.*\>/ ) {
	$host=$1;
    }
    if(/\<modules.*nmod=\"(\d+)\".*\>/ ) 
    {
	$nmod=$1;
	$nmod++;

	if( $jobid ne "" && $host ne "" ) {
	    $esrc = "$host"."_nodepower";

	    $estart = value_at_time(\%energy, "$esrc", $start);
	    $estop  = value_at_time(\%energy, "$esrc", $stop);

	    $consumed = $estop - $estart;

#	    print STDERR "**  '$start' '$stop' '$estart' '$estop'\n";

	    print "<modules nmod=\"$nmod\">\n";
	    print "<module name=\"Energy\" ".
		"consumed=\"$consumed\" unit=\"Joules\" ".
		"source=\"$esrc\"></module>\n";

	    $host="";
	} else {
	    print "$_\n";
	}
    } else {
	print "$_\n";
    }
}


sub read_powerdata
{
    my $ref      = shift @_;
    my $slurmid  = shift @_;   # slurm-id
    
    my $source;
    my $tv;
    my $pow;

    my $command = "lcpower $slurmid";
    open(CMD,"$command |") || die "Failed: $!\n";

    #open(CMD,"lcpower.dat") || die "Failed: $!\n";
    while(<CMD>)
    {
	if( $_ =~ /Error.*/ ) 
	{
	    die "Error connecting to Power Database with command '$command'.\n";
	}
	chomp;
	if( $_ =~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d),(\w+),([\d\.]+)/ ) 
	{
	    $year = $1;
	    $mon  = $2;
	    $day  = $3;
	    $hour = $4;
	    $min  = $5;
	    $sec  = $6;
	    
	    $source=$7; # e.g., lxa65_nodepower
	    $pow=$8;
	    
	    $tv = timelocal($sec, $min, $hour, $day, $mon-1, $year);
	    
	    ${$ref->{$source}}{$tv} = $pow;
	}
    }  

    for $source (keys %{$ref}) 
    {
	@keys = sort keys %{ $ref->{$source} };
	
	$tfirst = $keys[0];
	$tlast  = $keys[-1];
	
	${$ref->{$source}}{$tfirst-60} = ${$ref->{$source}}{$tfirst};
        ${$ref->{$source}}{$tlast+60} = ${$ref->{$source}}{$tlast};

    }
}

sub power_to_energy
{
    my $powr = shift @_; # power db
    my $ener = shift @_; # energy db

    my $source;
    my $tlast;
    my $time;
    
    $tlast=-1;
    for $source (keys %{$powr}) 
    {
	for $time (sort keys %{ $powr->{$source} } ) {

	    if( $tlast>0 ) {
		$en = 
		    ${$ener->{$source}}{$tlast} + 
		    ($time - $tlast) * 
		    (${$powr->{$source}}{$time} + ${$powr->{$source}}{$tlast})/2;
	    } else {
		$en = 0.0;
	    }
	    
	    #print "$source $time $en\n";
	    	    
	    ${$ener->{$source}}{$time} = $en;
	    $po = ${$powr->{$source}}{$time};
	    
	    $tlast = $time;
	}
    }
}



sub value_at_time
{
    my $ref    = shift @_;
    my %values = %{$ref};

    my $source = shift @_;
    my $when   = shift @_;

    my $time; my $tlast;
    my $val=0.0;
    my $v1; my $v2;

    if( scalar keys %{ $values{$source}} == 0 ) 
    {
	die "No power entries for $source!\n";
    }
    
    $tlast=-1;
    for $time (sort keys %{ $values{$source} } ) {
	if( $tlast<0 ) {
	    if( $when < $time ) {
		$val=${values{$source}}{$time};
#		printf("time $when is too small first=$time\n");
		return $val;
	    }
	    
	    $tlast=$time;
	    next;
	}
	
	if( ($tlast <= $when) && ($when < $time) ) 
	{
	    $v1 = ${values{$source}}{$tlast};
	    $v2 = ${values{$source}}{$time};
	    
	    $val = ${values{$source}}{$tlast} + 
		($v2 - $v1) * ($when-$tlast)/($time-$tlast);
	    
#	    printf "** found interval for $when: [$tlast,$time) $val $v1 $v2**\n";
	}
	
	$tlast=$time;
    }
    if( $when >= $tlast ) {
	$val=${values{$source}}{$tlast};
#	printf("time $when is too large tlast=$tlast\n");
	return $val;
    }
    
    return $val;
}

