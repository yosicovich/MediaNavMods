#!perl -w
# (C) 2003-2007 Willem Jan Hengeveld <itsme@xs4all.nl>
# Web: http://www.xs4all.nl/~itsme/
#      http://wiki.xda-developers.com/
#
# $Id$
#
use strict;
use IO::File;
use Getopt::Long;
use Dumpvalue;
my $d= new Dumpvalue;
$|=1;
my $verbose=0;
GetOptions(
    "v+"=>\$verbose,
);
if (@ARGV!=2) { die "Usage: fdf2reg <infile> <outfile>\n"; }
my $fdffile= shift;
my $regfile= shift;
my $in= IO::File->new($fdffile, "r") or die "$fdffile: $!\n";
binmode $in;

my $out= IO::File->new($regfile, "w+") or die "$regfile: $!\n";

# types: 0=NONE, 1=SZ, 2=EXPAND_SZ, 3=BINARY, 4=DWORD | DWORD_LE, 5=DWORD_BE,
#        6=LINK, 7=MULTI_SZ, 8=RESOURCE_LIST, 9=FULL_RESOURCE_DESCRIPTOR,
#       10=RESOURCE_REQUIREMENTS_LIST, 11=QWORD | QWORD_LE
my @typenames= qw(NONE SZ EXPAND_SZ BINARY DWORD DWORD_BE LINK MULTI_SZ RESOURCE_LIST FULL_RESOURCE_DESCRIPTOR RESOURCE_REQUIREMENTS_LIST QWORD);
my @typeprinters= map { eval "\\&print$_" } @typenames;
$typeprinters[21]=\&printMUI_SZ;

my @shorthive= qw(HKCR HKCU HKLM HKU);
my @longhive= qw(HKEY_CLASSES_ROOT HKEY_CURRENT_USER HKEY_LOCAL_MACHINE HKEY_USER);

my $filetypeversion= 0;

my $sig= readdword($in); # b274 831d

if ($sig!=0x1d8374b2) {
    die "unknown fdf file signature\n";
}
my $size= readdword($in);
#printf("%08lx %08lx\n", $sig, $size);

$out->print("REGEDIT4\n\n");

while (!$in->eof)
{
    my $ofs= $in->tell();
	my $recsize= readword($in);
	my $rectype= readword($in);
    printf("%08lx: %04x %04x", $ofs, $recsize, $rectype) if ($verbose>1);
	if ($rectype==1) {
        my $valtype= readword($in);
        my $keylen= readword($in);
        printf(" %04x %04x", $valtype, $keylen) if ($verbose>1);
        if ($filetypeversion==3) {
            my $datalen= readword($in);
            printf(" %04x", $datalen) if ($verbose>1);
            if ($datalen!=0) {
                die sprintf("data specified with hive key at %08lx\n", $ofs);
            }
            if (2*$keylen+3*2 != $recsize) {
                die sprintf("record size incorrect at %08lx\n", $ofs);
                next;
            }
        }
        elsif ($filetypeversion==4) {
            if (2*$keylen+2*2 != $recsize) {
                die sprintf("record size incorrect at %08lx\n", $ofs);
                next;
            }
        }
        else {
            # determine filetype version
            if (2*$keylen+2*2 == $recsize) {
                $filetypeversion= 4;
            }
            elsif (2*$keylen+3*2 == $recsize) {
                $filetypeversion= 3;
                my $datalen= readword($in);    # first time datalen is read here
                printf(" %04x", $datalen) if ($verbose>1);
                if ($datalen!=0) {
                    die sprintf("data specified with hive key at %08lx\n", $ofs);
                }
            }
        }

        # todo: check if key contains unescaped backslashes.
        my $key= readwstring($in, $keylen);
        printf(" %s\n", $key) if ($verbose>1);


        if (!$longhive[$valtype]) { 
            die sprintf("unknown hive type %04x at %08lx\n", $valtype, $ofs);
            next; 
        }

		$out->printf("\n[%s\\%s]\n", $longhive[$valtype], $key);
	}
	elsif ($rectype==2) {
        my $valtype= readword($in);
        my $keylen= readword($in);
        my $datalen= readword($in);

        my $key= readwstring($in, $keylen);
        my $data= readdata($in, $datalen);

        printf(" %04x %04x %04x %s\n", $valtype, $keylen, $datalen, $key) if ($verbose>1);
        printf("%s\n", unpack("H*", $data)) if ($verbose>1);

        if ($datalen+2*$keylen+3*2 != $recsize) {
            die sprintf("record size incorrect at %08lx\n", $ofs);
            next;
        }


        my $keystr= quotestr($key);
        my $pos= length($keystr)+1;
        if ($valtype > $#typeprinters || !$typeprinters[$valtype]) {
            $out->printf("%s=%s\n", $keystr, DefaultPrinter($valtype, $data, $pos));
        }
        else {
            $out->printf("%s=%s\n", $keystr, $typeprinters[$valtype]($data, $pos) || DefaultPrinter($valtype, $data, $pos));
        }
	}
    else {
        warn sprintf("unknown recordtype %d at %08lx\n", $rectype, $ofs);
        next;
    }
}
$out->close();
$in->close();

######################## reading functions ###############################
sub readdword {
	my $fh= shift;
	my $data;
	$fh->read($data, 4);
	return unpack("V", $data);
}

sub readword {
	my $fh= shift;
	my $data;
	$fh->read($data, 2);
	return unpack("v", $data);
}
sub readwstring {
	my $fh= shift;
	my $len= shift;
	my $data;
	$fh->read($data, 2*$len);
	return unpack("A*", pack("U*", unpack("v*", $data)));
}
sub readdata {
	my $fh= shift;
	my $len= shift;

	return "" if ($len==0);

	my $data;
	$fh->read($data, $len);
	return $data;
}

######################## printing functions ###############################
sub DefaultPrinter {
    my ($type, $data, $pos)= @_;
    return sprintf("hex(%x):", $type).hexdump($data, $pos+7);
}
sub printNONE {
	my ($data)= @_;
    return undef;
}
sub printSZ {
	my ($data)= @_;
    return data2wstring($data);
}
sub printMUI_SZ {
	my ($data)= @_;
    return "mui_sz:".data2wstring($data);
}
sub printEXPAND_SZ {
	my ($data)= @_;
    return undef;
}
sub printBINARY {
	my ($data, $pos)= @_;
    return "hex:".hexdump($data, $pos+4);
}
sub printDWORD {
	my ($data)= @_;
    return undef if (length($data)!=4);
    return sprintf("dword:%08lx", unpack("V", $data));
}
sub printDWORD_BE {
	my ($data)= @_;
    return undef;
}
sub printLINK {
	my ($data)= @_;
    return undef;
}
sub printMULTI_SZ {
	my ($data)= @_;
    return "multi_sz:".join ",\\\n  ", map { quotestr($_); } data2stringlist($data);
}
sub printRESOURCE_LIST {
	my ($data)= @_;
    return undef;
}
sub printFULL_RESOURCE_DESCRIPTOR {
	my ($data)= @_;
    return undef;
}
sub printRESOURCE_REQUIREMENTS_LIST {
	my ($data)= @_;
    return undef;
}

################ data formatting #######################################
sub hexdump {
    my ($data, $pos)= @_;
    my @bytes= unpack("C*", $data);
    my @lines= ();
    my $str= "";
    for (@bytes) {
        $str .= "," if ($str);
        if (length($str)+5 >=80-$pos) {
            push @lines, $str;
            $str= "";
            $pos= 0;
        }
        $str .= sprintf("%02x", $_);
    }
    push(@lines, $str) if ($str);
    return join "\\\n  ", @lines;
}
sub data2wstring {
	my $data= shift;

	return quotestr(unpack("A*", pack("U*", unpack("v*", $data))));
}
sub quotestr {
	my ($str)= @_;
	$str =~ s/(["\\])/\\$1/g;
	$str =~ s/\x0d/\\r/g;
	$str =~ s/\x0a/\\n/g;
	$str =~ s/\x00/\\0/g;
	$str =~ s/[\x00-\x1f\x7f-\xff]/sprintf("\\x%02x", ord($&))/ge;

	return qq("$str");
}
sub data2stringlist {
	my $data= shift;

	return split /\0/, pack("U*", unpack("v*", $data));
}
