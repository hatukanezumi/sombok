#-*- perl -*-

use version;

my $vernum = version->new($ARGV[1])->numify;

if ($ARGV[0] eq 'lb') {
    goto LB_CUSTOM;
} elsif ($ARGV[0] eq 'ea') {
    goto EA_CUSTOM;
} elsif ($ARGV[0] eq 'gb') {
    goto GB_CUSTOM;
} else {
    exit 0;
}

LB_CUSTOM:

print <<'EOF';
## SA characters may be categorized by their Grapheme_Cluster_Break properties.
## See [UAX #29].
EOF

exit 0;

EA_CUSTOM:

open UD, '<', "UnicodeData-$ARGV[1].txt";
while (<UD>) {
    ($code, $name, $cat) = split /;/;
    if ($cat =~ /^(Me|Mn|Cc|Cf|Zl|Zp)$/) {
	print "$code;Z # $name\n";
    }
}
close UD;
exit 0;

GB_CUSTOM:

exit 0 unless 6.001000 <= $vernum;

my @codes;
my %Virama;
my %GC_Letter;
my %Brahmic_Script;
my %Brahmic;

open my $ucd, '<', "UnicodeData-$ARGV[1].txt" or die $!;
while (<$ucd>) {
    chomp $_;
    s/\s*#.*$//;
    next unless /\S/;
    my ($code, $name, $gc, $ccc) = split /;/;
    $code = hex("0x$code");
    $Virama{$code} = 1 if $ccc+0 == 9;
    $GC_Letter{$code} = 1 if $gc =~ /^L/;
    push @codes, $code;
}
close $ucd;

open my $scr, '<', "Scripts-$ARGV[1].txt" or die $!;
while (<$scr>) {
    s/\s*\#.*//;
    next unless /\S/;

    my ($char, $prop) = split /\s*;\s*/, $_;
    chomp $prop;
    next unless $prop =~ /^(\@[\w:]+|\w+)$/;
    my ($start, $end) = ();
    ($start, $end) = split /\.\./, $char;
    $end ||= $start;
    foreach my $c (hex("0x$start") .. hex("0x$end")) {
        $Brahmic_Script{$prop} = 1 if $Virama{$c};
    }
}
close $scr;
open $scr, '<', "Scripts-$ARGV[1].txt" or die $!;
while (<$scr>) {
    s/\s*\#.*//;
    next unless /\S/;

    my ($char, $prop) = split /\s*;\s*/, $_;
    chomp $prop;
    next unless $prop =~ /^(\@[\w:]+|\w+)$/;
    my ($start, $end) = ();
    ($start, $end) = split /\.\./, $char;
    $end ||= $start;
    foreach my $c (hex("0x$start") .. hex("0x$end")) {
	$Brahmic{$c} = 1 if $Brahmic_Script{$prop};
    }
}
close $scr;

open my $gcb, '<', "GraphemeBreakProperty-$ARGV[1].txt" or die $!;
while (<$gcb>) {
    s/\s*\#.*//;
    next unless /\S/;

    my ($char, $prop) = split /\s*;\s*/, $_;
    chomp $prop;
    next unless $prop =~ /^(\@[\w:]+|\w+)$/;

    my ($start, $end) = ();
    ($start, $end) = split /\.\./, $char;
    $end ||= $start;
    foreach my $c (hex("0x$start") .. hex("0x$end")) {
	$prop{$c} = $prop;
    }
}
close $gcb;

foreach my $c (@codes) {
    my $prop = $prop{$c};

    # Custom GCB Virama
    if ($Virama{$c}) {
	if ($prop =~ /^(Extend|SpacingMark)$/) {
	    $prop = 'Virama';
	} else {
	    die sprintf "U+%04X is virama and %s", $c, $prop;
	}
    } elsif ($GC_Letter{$c} and $Brahmic{$c}) {
	if ($prop) {
	    warn sprintf
		"(non-fatal) U+%04X: GB=%s; won't assign OtherLetter.\n",
		$c, $prop;
	} else {
	    $prop = 'OtherLetter';
	}
    }

    printf "%04X ; %s\n", $c, $prop unless $prop eq $prop{$c};
}


exit 0;

