#!/usr/bin/env perl
use strict;
use warnings;
use IO::Handle;
use IPC::Open3;
use POSIX qw<dup2>;
use File::Spec::Functions;

my $clang_path = shift @ARGV;

my ($p_r, $p_w);
pipe($p_r, $p_w)
  or die $!;

my $chld = fork;
if ($chld) {
    $p_w->close();
    my $f = 0;
    while (defined (my $line = <$p_r>)) {
        if (!$f && substr($line,0,length($clang_path)+1) eq " $clang_path") {
            chomp $line;
            STDOUT->print(substr($line,length($clang_path)+1));
            $f = 1;
        }
    }
    $p_r->close();
    waitpid($chld,0);
    if ($? != 0) {
        die "Suprocess died with value $?";
    }
} else {
    $p_r->close();
    dup2(fileno($p_w), fileno(STDERR))
      or die $!;
    open STDOUT, '>', '/dev/null';
    open STDIN, '<', '/dev/null';
    my @cmd = (
        catfile($clang_path, 'bin/clang'),
        '-v', '-xc', '-c', '/dev/null', '-o', '/dev/null'
    );
    exec @cmd
      or die $!;
}
