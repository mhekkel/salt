#!/usr/bin/perl

use strict;
use warnings;

use Digest::SHA1 qw(sha1_hex);
use Math::BigInt;
use MIME::Base64;
use Getopt::Std;
use Data::Dumper;

my %opts;
getopt("n:s:", \%opts);

print Dumper(\%opts);

my $name = $opts{n};
my $sqnm = $opts{s};

print "n: $name\ns: $sqnm\n";
$name = 'M.L. Hekkelman'	unless defined $name;
$sqnm = 4					unless defined $sqnm;

print "n: $name\ns: $sqnm\n";

my $hs = sha1_hex($name);
$hs = "0x$hs" unless $hs =~ m/^0x/;
my $h = Math::BigInt->from_hex($hs);

my $m = Math::BigInt->new(35);
$m = $m->bpow(12);

my $c = $h->bmodpow($sqnm, $m);

my $code = "";
for (my $i = 0; $i < 12; ++$i) {
	my $d = Math::BigInt->new($c);
	
	my $ch = $d->bmod(35);
	
	if ($ch >= 10) {
		$ch = chr(ord('@') + $ch - 9);
	}
	else {
		$ch = chr(ord('0') + $ch);
	}
	
	$code = $ch . $code;
	
	$c->bdiv(35);
}

#my $code = substr($c->as_hex, 2);
$code = '0' x (12 - length($code)) . $code
	unless length($code) == 12; 
$code =~ s/.{4}/$&-/g;
$code = $code . sprintf("%4.4d", $sqnm);
print lc "$code\n";
