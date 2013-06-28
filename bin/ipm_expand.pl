#!/usr/bin/perl

# expanding clustered XML profiles to "normal" full ones for use with 
# the standard parser. 

%tasks=();
%taskcopies=();

$rank=-1;
$nprocs=-1;

while(<>) {
    $line = $_;

    chomp $line;

    if( /<task .*mpi_rank=\"(\d+)\"/ ) {
	$rank=$1;
    }

    if( $nprocs==-1 && /mpi_size=\"(\d+)\"/ ) {
	$nprocs=$1;
    }


    if( /<\/task>/ ) {
	push @{ $tasks{$rank} }, $line;
	$rank=-2;
    }

    if( /<taskcopy .*mpi_rank=\"(\d+)\" cluster_rank=\"(\d+)\"/ ) {
	$taskcopies{$1} = $2;
    }

    if( $rank >= -1 ) {
	push @{ $tasks{$rank} }, $line;
    }
}


# print header...
foreach $line (@{ $tasks{-1} }) {
    print "$line\n";    
}


for( $rank=0; $rank<$nprocs; $rank++ ) {
    if( defined( $taskcopies{$rank} ) ) {

	$clust =  $taskcopies{$rank};
	
	foreach $line (@{ $tasks{$clust} }) {

	    $pline = $line;

	    if( $pline =~ /mpi_rank=\"$clust\"/ ) {
		$pline =~ s/mpi_rank=\"$clust\"/mpi_rank=\"$rank\"/g;
	    }

	    if( $pline =~ /orank=\"\+(\d+)\"/ ) {
		$orank=$1+$rank;
		$pline =~ s/orank=\"\+(\d+)\"/orank=\"$orank\"/;
	    }
	    if( $pline =~ /orank=\"\-(\d+)\"/ ) {
		$orank=$rank-$1;
		$pline =~ s/orank=\"\-(\d+)\"/orank=\"$orank\"/;
	    }
	    print "$pline\n";
	}
    } else {
	foreach $line (@{ $tasks{$rank} }) {

	    $pline = $line;

	    if( $pline =~ /orank=\"[\+](\d+)\"/ ) {
		$orank=$1+$rank;
		$pline =~ s/orank=\"[\+](\d+)\"/orank=\"$orank\"/;
	    }
	    if( $pline =~ /orank=\"[\-](\d+)\"/ ) {
		$orank=$rank-$1;
		$pline =~ s/orank=\"[\-](\d+)\"/orank=\"$orank\"/;
	    }
	    print "$pline\n";
	}
    }
}

#print footer
print "</ipm_job_profile>\n";
