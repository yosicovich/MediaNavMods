#!perl -w
# (C) 2003-2007 Willem Jan Hengeveld <itsme@xs4all.nl>
# Web: http://www.xs4all.nl/~itsme/
#      http://wiki.xda-developers.com/
#
# $Id$
#
# todo: add support for rgu file features:
#    - comments
#    - linecontinuation with "\s*\\\n\s*" sequence
#    - linecontinuation with "\s*,\n\s*" sequence
#    - deletekey  : \[-keyname\]
#    - deletevalue : valname=-
#    - unquoted keyname
use strict;
use IO::File;
use Getopt::Long;

my %escmap= ( 0=>"\0", n=>"\n", r=>"\r", t=>"\t" );

my @shorthive= qw(HKCR HKCU HKLM HKU);
my @longhive= qw(HKEY_CLASSES_ROOT HKEY_CURRENT_USER HKEY_LOCAL_MACHINE HKEY_USER);

my %hivemap= map { $longhive[$_] => $_ } (0..$#longhive);

my @typenames= qw(NONE SZ EXPAND_SZ BINARY DWORD DWORD_BE LINK MULTI_SZ RESOURCE_LIST FULL_RESOURCE_DESCRIPTOR RESOURCE_REQUIREMENTS_LIST QWORD);
my %typemap= map { $typenames[$_] => $_ } (0..$#typenames);
$typemap{MUI_SZ}= 21;

# translates 'hex(7)' to multi_sz
# translates valuename '@' to 'Default'
my $g_filetypeversion;
my $g_fixedsize;

GetOptions(
    "4" => sub { $g_filetypeversion=4; },
    "3" => sub { $g_filetypeversion=3; },
    "s=s" => sub { $g_fixedsize= eval($_[1]) },
) or die usage();
if (@ARGV!=2) { die usage(); }

sub usage {
    return "Usage: reg2fdf [-3 | -4 ] <infile> <outfile>\n";
}
$g_filetypeversion ||= 3; # default to version 3

my $registry= ReadRegFile(shift);

WriteFdfFile(shift, $registry);

printf("created wince %d.x compatible fdf file\n", $g_filetypeversion);

exit(0);

sub WriteFdfFile {
    my ($fn, $reg)= @_;

    my $out= IO::File->new($fn, "w+") or die "$fn: $!\n";
    binmode $out;

    $out->print(pack("VV", 0x1d8374b2, 0));

    my $lastpath;
    for (my $i=0 ; $i<@$reg ; $i++)
    {
        my $regkey= $reg->[$i];
        if ($g_filetypeversion==3) {
            WriteRegpath_v3($out, ParseRegPath($regkey->{path}));
        }
        elsif ($g_filetypeversion==4) {
            WriteRegpath_v4($out, ParseRegPath($regkey->{path}));
        }
        else {
            die "unknown version\n";
        }

        for (my $j=0 ; $j<@{$regkey->{values}} ; $j++)
        {
            my $value= $regkey->{values}[$j];
            WriteRegKeyValue($out, $value->{name}, $value->{type}, $value->{value});
        }
    }
    if ($g_fixedsize) {
        my $regpath= "HKEY_LOCAL_MACHINE\\Software\\padding";

        if ($g_filetypeversion==3) {
            WriteRegpath_v3($out, ParseRegPath($regpath));
        }
        elsif ($g_filetypeversion==4) {
            WriteRegpath_v4($out, ParseRegPath($regpath));
        }
        # minimum keyval entry
        #   10 + len(valuename) + len(value)  : valuename= 1char -> 12
        if ($g_fixedsize < ($out->tell()) + 12) {
            warn "WARNING: file larger than specified fixed size\n";
        }

        my $needed= $g_fixedsize - $out->tell();
        my $n_fullentries= int($needed/256);
        if (($needed%256)>0 && ($needed%256)<12) {
            # we would end up with a chunk too small for 1 kv.
            # -> divide this up over the last 2 keyvals.
            $n_fullentries--;
        }
        for (my $i=0 ; $i< $n_fullentries ; $i++) {
            WriteFillerKeyVal($out, UnQuoteStr("$i"), 256);
            $needed -= 256;
        }
        if ($needed > 256) {
            WriteFillerKeyVal($out, UnQuoteStr("a"), int($needed/2));
            $needed -= int($needed/2);
        }
        WriteFillerKeyVal($out, UnQuoteStr("b"), $needed);
    }

    my $filesize= $out->tell();

    $out->seek(4, SEEK_SET);
    $out->print(pack("V", $filesize));
    $out->seek(0, SEEK_END);
    $out->close();
}
sub WriteFillerKeyVal {
    my ($out, $name, $vallen)= @_;

    WriteRegKeyValue($out, $name, $typemap{BINARY}, "\x00" x ($vallen-10-length($name)));
}
sub ReadRegFile {
    my ($fn)= @_;

    my @reg;

    my $in= IO::File->new($fn, "r") or die "$fn: $!\n";

    my ($curvaluename, $curvaluestring);
    my $curkey;

    while(<$in>)
    {
        s/\s+$//;

        if (/^REGEDIT4$/ || /^$/ || /^;/) {
        # header line
        }
        elsif (/^\[(.*)\]$/) {
        # line with path
            $curkey= { path=>$1, values=>[] };
            push @reg, $curkey;
        }
        elsif (/^(?:"((?:[^"\\]|\\["\\0nrt]|\\x\w\w)*)"|@)\s*=\s*(.*)/)
        {
        # line with quoted valuename
            my ($name, $value)= ($1, $2);
            if (!defined $name) {
                $name= "Default";
            }
            if ($value =~ /(.*)\\$/) {
                $curvaluename= $name;
                $curvaluestring= $1;
            }
            else {
                push @{$curkey->{values}}, {
                    name=>UnQuoteStr($name),
                    ParseValue($value),
                };
            }
        }
        elsif (/^\s+(.*)/) {
        # continued line
            my ($value)= ($1);
            if ($value =~ /(.*)\\$/) {
                $curvaluestring .= $1;
            }
            else {
                $curvaluestring .= $value;
                push @{$curkey->{values}}, {
                    name=>UnQuoteStr($curvaluename),
                    ParseValue($curvaluestring),
                };
            }
        }
        else {
            die "unrecognised line:\n$_\n";
        }
    }
    $in->close();

    return \@reg;
}

# regpath is not escaped.
sub ParseRegPath {
    my ($regpath)= @_;
    # returns (hivenr, path)
    if ($regpath =~ /^(\w+)(?:\\(.*))?$/) {
        if (!exists $hivemap{$1}) { die "unknown hive $1\n"; }
        else { return ($hivemap{$1}, $2); }
    }
    else {
        die "reg path unexpected format $regpath\n";
    }
}

sub UnQuoteStr {
    my ($str)= @_;

    return undef unless (defined $str);

    $str =~ s/\\(?:x(\w\w)|(.))/defined $1 ? chr(hex($1)) : exists $escmap{$2} ? $escmap{$2}:$2/ge;

    return pack("v*", unpack("U*", $str), 0);
}
sub ParseValue {
    my ($valuestr)= @_;

    if ($valuestr =~ /^dword:(\w+)$/i) {
        return (type=>$typemap{DWORD}, value=>ParseDword($1));
    }
    elsif ($valuestr =~ /^mui_sz:"((?:[^"\\]|\\["\\0nrt]|\\x\w\w)*)"$/i) {
        return (type=>$typemap{MUI_SZ}, value=>ParseString($1));
    }
    elsif ($valuestr =~ /^multi_sz:(.*)$/i) {
        return (type=>$typemap{MULTI_SZ}, value=>ParseMultiString($1));
    }
    elsif ($valuestr =~ /^hex:(.*)$/i) {
        return (type=>$typemap{BINARY}, value=>ParseBinary($1));
    }
    elsif ($valuestr =~ /^hex\((\w)\):(.*)$/i) {
        return (type=>hex($1), value=>ParseBinary($2));
    }
    elsif ($valuestr =~ /^"((?:[^"\\]|\\["\\0nrt]|\\x\w\w)*)"$/) {
        return (type=>$typemap{SZ}, value=>ParseString($1));
    }
    else {
        die "Unknown datatype : $valuestr\n";
    }
}
sub ParseBinary {
    my ($str)= @_;

    $str =~ s/,//g;
    return pack("H*", $str);
}
sub ParseString {
    my ($str)= @_;
    # !!! unquotestr also converts to unicode and adds NUL.
    return UnQuoteStr($str);
}
sub ParseMultiString {
    my ($str)= @_;

    my @list;
    my $pos= 0;
    while (defined $pos) {
        pos($str)= $pos;
        if ($str =~ /\G"((?:[^"\\]|\\["\\0nrt]|\\x\w\w)*)"/gs) {
            $pos= pos($str);
            push @list, ParseString($1);
        }
        else {
            last;
        }
        pos($str)= $pos;
        if ($str =~ /\G,/gs) {
            $pos= pos($str);
        }
        else {
            last;
        }
    }

    return join("", @list). "\x00\x00";
}
sub ParseDword {
    my ($str)= @_;
    return pack("V", hex($str));
}
sub WriteRegKeyValue {
    my ($fh, $name, $type, $value)= @_;

    my $recsize= length($value)+length($name)+6;
    $fh->print(pack("v5", $recsize, 2, $type, length($name)/2, length($value)));
    $fh->print($name); #pack("v*", unpack("U*", ))
    $fh->print($value);
}
sub WriteRegpath_v3 {
    my ($fh, $hive, $path)= @_;

    my $recsize= 2*length($path)+6+2;
    $fh->print(pack("v5", $recsize, 1, $hive, length($path)+1, 0));
    $fh->print(pack("v*", unpack("U*", $path), 0));
}
sub WriteRegpath_v4 {
    my ($fh, $hive, $path)= @_;

    my $recsize= 2*length($path)+4+2;
    $fh->print(pack("v4", $recsize, 1, $hive, length($path)+1));
    $fh->print(pack("v*", unpack("U*", $path), 0));
}
