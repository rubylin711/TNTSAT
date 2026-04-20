if the reversion of aclocal that am__api_version in configure is not same with your system,
build will error like this:

CDPATH="${ZSH_VERSION+.}:" && cd /home/lujiang/work/mt_sdk/symphony/linux/component/wifi/tools/libnl/libnl-3.2.25 && /bin/bash /home/lujiang/work/mt_sdk/symphony/linux/component/wifi/tools/libnl/libnl-3.2.25/build-aux/missing aclocal-1.13 -I m4
/home/lujiang/work/mt_sdk/symphony/linux/component/wifi/tools/libnl/libnl-3.2.25/build-aux/missing: line 81: aclocal-1.13: command not found
WARNING: 'aclocal-1.13' is missing on your system.
         You should only need it if you modified 'acinclude.m4' or
         'configure.ac' or m4 files included by 'configure.ac'.
         The 'aclocal' program is part of the GNU Automake package:
         <http://www.gnu.org/software/automake>
         It also requires GNU Autoconf, GNU m4 and Perl in order to run:
         <http://www.gnu.org/software/autoconf>
         <http://www.gnu.org/software/m4/>
         <http://www.perl.org/>
make[6]: *** [/home/lujiang/work/mt_sdk/symphony/linux/component/wifi/tools/libnl/libnl-3.2.25/aclocal.m4] Error 127



in this case, you should run the following command:
1, sudo apt-get install autoscan aclocal autoconf autoheader automake
2, cd libnl-3.2.25
3, autoreconf -f -i
then you can build it

please don't git add/commit the changed file, because other people's system may be not same with yours.
