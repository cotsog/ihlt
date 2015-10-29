use Test::More;
use Devel::GDB;

my $gdb = new Devel::GDB('-params' => 'src/ihlt');
my $t = $gdb->get('info functions');
open my ($str_fh), '<', \$t;
1
while(diag <$str_fh>);
plan skip_all => 'No tests.';
