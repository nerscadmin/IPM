#!/usr/bin/perl -w

#
# ipm2txt.pl - Contributed by Evgeniy German (evgeniyge@voltaire.com)
# 
# Usage

# ipm2txt.pl -xmlfile /path/to/ipm_output.xml -outfile /path/to/report.xls [-bin number]
#
# This generates 3 new reports in the single excel file:
#
#	- msg bin distribution for pt2pt communication
#	- msg bin distribution for pt2pt communication for msg > over user provided limit
#	- msg bin distribution for pt2pt communication, sorted by total bytes to identify top talking ranks
#

use XML::Parser;
use List::Util qw[min max];
use Getopt::Long;
use Switch;
use Spreadsheet::WriteExcel;



sub usage (){
    print ("./Ipm2txt.pl -xmlfile somexmlfile.xml -outfile results.csv [-bin 512 ]\n");
}
my $xmlfile;
my $out_file;
my $bin = 0;
GetOptions( 'xmlfile=s' => \$xmlfile, 'outfile=s' => \$out_file, 'bin=i'=>\$bin) or die "Incorrect usage!\n";

if(!$xmlfile or ! -e $xmlfile ) {
    print ("Can't find xml file or wrong usage\n");
    usage();
    exit;
}

if(!$out_file) {
    $out_file="/tmp/out_file.xls";
}
if($out_file!~/.xls/) {
    $out_file=$out_file.".xls";
}

if(defined ($bin) && $bin=~/\D+/) {
    print ("bin should be only number in bytes");
    exit;
}

my $workbook = Spreadsheet::WriteExcel->new($out_file);
my $worksheet = $workbook->add_worksheet("General Data");
my $worksheet_sort=$workbook->add_worksheet("Sort by TotalBytes");
my $worksheet_bin;
my $bin_y;
if(defined ($bin)) {
   $worksheet_bin=$workbook->add_worksheet("Bin");
}
my $format = $workbook->add_format();
$format->set_bold();
my @arr_names=("SRank","DRank","Messages","(minsize-maxsize)/[bin]","Totalbytes");
$worksheet->write_string(0,0,"Message Summary by Rank Pair:",$format);
if(defined ($bin)) {
    $worksheet_bin->write_string(0,0,"all msg > $bin",$format);
}


my %sort_Messages;
my %sort_Messages_other_info;
$worksheet_sort->write_string(0,0,"Sorted by TotalBytes:",$format);
my $y=1;
my $x=0;
foreach my $name (@arr_names) {
    $worksheet->write_string($y,$x,"$name",$format);
    $worksheet_sort->write_string($y,$x,"$name",$format);
    if(defined ($bin)) {
        $worksheet_bin->write_string($y,$x,"$name",$format);
    }
    $x++;
}

 $y++;
 $x=0;
 $bin_y=2;
 #open DATA ,">$out_file" or die $!;
 #print DATA "Message Summary by Rank Pair:\n\n";
 #print DATA "SRank,DRank,Messages,(minsize-maxsize)/[bin],Totalbytes\n";
 #close DATA;
 
 my (%count,%bytes,%region,%orank,%call);
 my $parser = XML::Parser->new( Handlers => 
                                     {
                                      Start=>\&handle_start,
                                      End=>\&handle_end,
                                     });

$parser->parsefile( $xmlfile);
my $task;
# process a start-of-element 
#
sub sort_orank {
    $orank{$a}<=>$orank{$b};
}
sub sort_range_total {
    $range_total{$a}<=>$range_total{$b};
}

sub sort_total_bytes {
     $sort_Messages{$b}<=>$sort_Messages{$a};
}
sub Prepare_Bin{
    my %hash;
    my $x=2;
    for (my $j=4,my $a=0;$j<24;$j++) {
        my $tmp_ans=$x ** $j;
        $hash{$j}="$a-$tmp_ans";
        $a=$tmp_ans+1;
    }
    return %hash;
}
sub Check_Range{
    my ($range_total,$range,$bytes,$counts,%hash_range)=@_;
    my $count=0;
    while (my ($key,$value) =each %hash_range) {
        my ($min,$max)=split (/-/,$value);
        if(int($min) <= int ($bytes) && int ($bytes) <= int ($max)) {
            if(!$range->{$key}) {
                $range->{$key}=0;
            }
            if (!$range_total->{$key}) {
                $range_total->{$key}=0;
            }
            $range->{$key}+=$counts;
            $range_total->{$key}=$bytes*$counts+$range_total->{$key};
            $count++;  
        }
    }
}

sub write_to_file {
    #open WRITE,">>$out_file" or die $!;
    my (@arr_msg,@arr_bytes);
    my $Messages=0;
    my ($Minsize,$Max_size);
    my $total_size=0;
    my %range;
    my %hash_range=Prepare_Bin();
    my %range_total;
    my ($trank,$crank);
    foreach my $key (sort sort_orank (keys(%orank))) {
        $crank=$orank{$key};
        if(defined ($trank) && $crank ne $trank) {
            $Minsize = min(@arr_bytes);
            $Max_size = max(@arr_bytes);
            $worksheet->write_number($y,$x++,$task);
            $worksheet->write_number($y,$x++,$trank);
            $worksheet->write_number($y,$x++,$Messages);
            $worksheet->write_string($y,$x++,"\($Minsize..$Max_size\)");
            $worksheet->write_number($y,$x++,$total_size);
            if(defined ($bin)) {
                $x=0;
                $worksheet_bin->write_number($bin_y,$x++,$task);
                $worksheet_bin->write_number($bin_y,$x++,$trank);
                $worksheet_bin->write_number($bin_y,$x++,$Messages);
                $worksheet_bin->write_string($bin_y,$x++,"\($Minsize..$Max_size\)");
                $worksheet_bin->write_number($bin_y,$x++,$total_size);
                $bin_y++;
            }
            $sort_Messages{$key}=$total_size;
            $sort_Messages_other_info{$key}=$task.";".$trank.";".$Messages.";"."\($Minsize..$Max_size\)";
            $y++;
            #print WRITE "$task,$trank,$Messages,\($Minsize..$Max_size\),$total_size\n";
            foreach my  $key  ( sort {$a<=>$b} keys (%range_total)) {
                #print (WRITE ",,$range{$key},\[ $hash_range{$key}\],$range_total{$key}\n");
                $x=2;
                $worksheet->write_number($y,$x++,$range{$key});
                $worksheet->write_string($y,$x++,"\[ $hash_range{$key}\]");
                $worksheet->write_number($y,$x++,$range_total{$key});
                my ($tmp_min,$tmp_max)=split("-",$hash_range{$key});
                if(defined ($bin) && $tmp_max > $bin) {
                    $x=2;
                    $worksheet_bin->write_number($bin_y,$x++,$range{$key});
                    $worksheet_bin->write_string($bin_y,$x++,"\[ $hash_range{$key}\]");
                    $worksheet_bin->write_number($bin_y,$x++,$range_total{$key});
                    $bin_y++;
                }
               
                $y++;
            }
            $x=0;
            @arr_msg=();
            @arr_bytes=();
            $total_size=0;
            $Messages=0;
            %range_total=();
            %range=();
        }
        Check_Range(\%range_total,\%range,$bytes{$key},$count{$key},%hash_range);
        push(@arr_msg,$count{$key});
        push(@arr_bytes,$bytes{$key});
        $Messages+=$count{$key};
        $total_size=$total_size+($count{$key} * $bytes{$key});
        
        $trank=$crank;
    }
    
    #print (WRITE "-,"x5);
   # print (WRITE "\n");
   # close WRITE;
}



sub handle_start {
    my( $expat, $element, %attrs ) = @_;
   if($element eq "task") {
    $task=$attrs{"mpi_rank"};
   }
   if ($element eq "hent") {
     foreach my $key (keys(%attrs)) {
	 switch($key) {
        case "count" {$count{$attrs{"key"}}=$attrs{$key}}
        case "bytes" {$bytes{$attrs{"key"}}=$attrs{$key}}
        case "region" {$region{$attrs{"key"}}=$attrs{$key}}
        case "orank" {$orank{$attrs{"key"}}=$attrs{$key}}
        case "call" {$call{$attrs{"key"}}=$attrs{$key}}
       }
   }#end foreach
 
    
   }#end start hent
}
 
# process an end-of-element event
#
sub handle_end {
    my( $expat, $element ) = @_;
    if($element eq "hash"){
        write_to_file();
        #zero all hashes;
        %call=();
        %orank=();
        %region=();
        %bytes=();
        %count=();
    }

}

$y=1;
$x=0;
foreach my $key (sort sort_total_bytes (keys(%sort_Messages))) {
    $x=0;
    $y++;
    my @tmp_arr=split(";",$sort_Messages_other_info{$key});
    foreach my $var (@tmp_arr) {
       if($var!~/\(/) {
        $worksheet_sort->write_number($y,$x++,$var);
       }
       else
       {
        $worksheet_sort->write_string($y,$x++,$var);
       }
       
    }
  $worksheet_sort->write_number($y,$x++,$sort_Messages{$key}); 
}




print ("result in : $out_file \n");
$workbook->close();
