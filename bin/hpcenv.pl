#!/usr/bin/perl

$hostname = `hostname`;
$os = `uname -s`; 
$os =~ s/\n//g;

if( $os eq "Linux" ) {
    $res  = `grep CentOS /etc/issue`;
    if( $res =~ /CentOS/ ) {
	$os = "Linux-CentOS";
    }
}

$system = "$os";

if( $hostname =~ /^nid\d\d\d\d\d/ ) {
    $system = "Franklin";
}
if( $hostname =~ /^hopper/ ) {
    $system = "Hopper";
}
if( $hostname =~ /^kraken.*/ ) {
    $system = "Kraken";
}
if( $hostname =~ /ranger/ ) {
    $system = "Ranger";
}
if( $hostname =~ /^n\d\d\d\d.scs\d\d/ ) {
    $system = "Lawrencium";
}
if( $hostname =~ /^a01/ ) {
    $system = "hlrb2";
}
if( $hostname =~ /ipm2dev/ ) {
    $system = "ipm2dev";
}
if( $hostname =~ /turing/ ) {
    $system = "turing";
}
if( $hostname =~ /tesla/ ) {
    $system = "tesla";
}
if( $hostname =~ /cvrsvc/ ) {
    $system = "carver";
}
if( $hostname =~ /salzburg/ ) {
    $system = "salzburg";
}
if( $hostname =~ /^atv/ ) {
    $system = "atvcluster";
}
if( $hostname =~ /^lx/ ) {
    $system = "LRZ_Linux_Cluster";
}
if( $hostname =~ /cloud02/ ) {
    $system = "LMU";
}




print "$system\n";
