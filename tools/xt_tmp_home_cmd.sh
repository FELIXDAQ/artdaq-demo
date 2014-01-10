#! /bin/sh
 # This file (xt_tmp_home_cmd.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Jan  8, 2014. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: xt_tmp_home_cmd.sh,v $
rev='$Revision: 1.6 $$Date: 2014/01/10 03:53:32 $'

# defaults
term=xterm
term_opts=-e  # -ls   # want login shell -- NO, use -e bash (see comment below)
term_geom=
bash_opts=-l
CMD_STR=

env_opts_var=`basename $0 | sed 's/\.sh$//' | tr 'a-z-' 'A-Z_'`_OPTS
USAGE="\
   usage: `basename $0` [-h?] [options] <root>
examples: `basename $0` xt_tmp_home -c'ps aux|grep \$$' -c': hi' -c'ls -a'
          `basename $0` xt_tmp_home -c'::echo xxx' -c'history|tail -n5'
          `basename $0` xt_tmp_home -c'echo hi' -c':^sleep 5' -c'echo done'
          `basename $0` xt_tmp_home -c'echo hi' -g'-geom 80x44'
          $env_opts_var=-g80x44 `basename $0` xt_tmp_home -c'echo x'
          echo -e 'echo hi - ^C to abort;sleep 2\n:!loop' >|xt_tmp_home/loop;\\
          `basename $0` xt_tmp_home -c':!loop'

-t<term> --term=<term>  terminal program (dflt:$term)
-c<cmd>
-g<geom_option> ???

special 2 character sequences (as in -c':^sleep 5' above):
:^   just eval command (no echo, no hist insert)
::   just insert into the history
:!   \"command\" is actuall a file of command(s)
:,   echo and eval, no hist insert
"

# Process script arguments and options
eval env_opts=\${$env_opts_var-} # can be args too
eval "set -- $env_opts \"\$@\""
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`   && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`   && set --  "$rest" "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help= opt_v=0
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x-[^=]*\(=\)'` lev=`expr "x$op" : 'x-[^=]*=\(.*\)'`
        test -n "$leq"&&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        \?*|h*)     eval $op1chr; do_help=1;;
        v*)         eval $op1chr; opt_v=`expr $opt_v + 1`;;
        x*)         eval $op1chr; set -x;;
        t*|-term)   eval $reqarg; term=$1; shift;;
        -bash-opts) eval $reqarg; bash_opts=$1; shift;;
        g*)         eval $op1arg;
                    if expr "x$1" : 'x[0-9]*[x+-][0-9]' >/dev/null;then
                        term_geom="-geometry $1"
                    else
                        term_geom=$1
                    fi
                    shift;;
        c*)         eval $op1arg; test -z "$CMD_STR" && CMD_STR=$1 || CMD_STR=`echo "$CMD_STR";echo "$1"`;shift;;
        *)          echo "Unknown option -$op"; do_help=1;;
        esac
    else
        aa=`echo "$1" | sed -e"s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
test $# -ne 1 && do_help=1  # 1 required arg
test -n "${do_help-}" && echo "$USAGE" && exit

set -u # helps development
pseudo_home=$1

if [ ! -d "$pseudo_home" ];then
    echo "creating $pseudo_home"
    mkdir -p "$pseudo_home"
fi

# if pseudo_home == real_home
#   could check for hist injection/cmd execution signature
#   could prompt for (or just optionally) addition (add) stuff
#   to users bashrc

init_profile_file() # $1=profile_file
{
    cat >$1 <<-'EOF'
	test -n "${REALHOME-}" && HOME=$REALHOME
	cmd_str_sav=$CMD_STR; unset CMD_STR
	if   [ -r $HOME/.bash_profile ];then
	        . $HOME/.bash_profile
	elif [ -r $HOME/.bash_login ];then
	        . $HOME/.bash_login
	elif [ -r $HOME/.profile ];then
	        . $HOME/.profile
	elif [ -f /etc/bashrc ];then
	        . /etc/bashrc
	fi
	CMD_STR=$cmd_str_sav
	histchars='!^#' # fix the (2nd part of the) strangeness (next line)
	history -n      # strange: shouldn't have to do this. also, timestamps show up.
	EOF
    #
}

append_cmd_str_support() # $1=profile_file
{
    cat >>$1 <<-'EOF'
	# CMD_STR support
	hcmd() { history -s "$@"; echo "$@"; eval "$@"; }
	process_cmd_str()
	{   cpltcmd=
	    IFSsav=$IFS IFS='
	';  for cmd_ in $1;do IFS=$IFSsav
	        if cmd__=`expr "x${cmd_}x" : 'x\(.*\)\\\\x$'`;then
	            cpltcmd="$cpltcmd$cmd__"; continue
	        else
	            cpltcmd="$cpltcmd$cmd_"
	        fi
	        if   histonly=`expr "x$cpltcmd" : 'x::\(.*\)'`;then
	            history -s "$histonly"
	        elif nohist=`expr   "x$cpltcmd" : 'x:,\(.*\)'`;then
	            echo "$nohist"; eval "$nohist"
	        elif file=`expr     "x$cpltcmd" : 'x:!\(.*\)'`;then
	            xx=`cat $file`
	            process_cmd_str "$xx"  # recursive call
	        elif just_exe=`expr "x$cpltcmd" : 'x:^\(.*\)'`;then
	            eval "$just_exe"
	        else
	            hcmd "$cpltcmd"
	        fi
	        cpltcmd=
	    done
	}
	if [ -n "${CMD_STR-}" ];then
	    process_cmd_str "$CMD_STR"
	    unset CMD_STR
	fi
	EOF
    #
}

# pseudo_home_profile
if [ ! -f $pseudo_home/.bash_profile ];then
    # create new one
    init_profile_file      $pseudo_home/.bash_profile
    append_cmd_str_support $pseudo_home/.bash_profile
else
    : # check if current is 
fi

# Note: bash has a "--rcfile" option which could be used
# but, I really want login shell (not merely interactive) and
# I want to be able to do: ps aux | grep $$
# and see "-bash" (bash with a leading "-") show up.
# Note: it does make sense to use the --rcfile option with a "login shell"
#       And actually using: --bash-opts="-l --rcfile ..."
#       results in bash not starting.
#
# *** WELL, I should concede that I will not get the "-" to show up --
#    this whole thing relies on bash and it's history functionality
# I SHOULD NOT ASSUME THAT THE USER'S LOGIN SHELL IS BASH!
#   --> no xterm -ls, explicitly specify "bash" and use -l.
# I think going "interactive/non-login and specifying the rcfile which
# will do what a login shell does" is equivalent to the specifying a login
# shell. So, the choice is between:
#     a) a simple/short cmd line "bash -l" and messing with HOME
# and b) a longer cmd line
# If I experience problem with "messing with HOME", I'll switch to using
# --rcfile

# special (mainly testing)
#  xt_tmp_home_cmd.sh xt_tmp_home -c'ps aux|grep $$' --bash-opts=
test -z "$bash_opts" -a "$term" = xterm && bash= term_opts=-ls || bash=bash
#  xt_tmp_home_cmd.sh xt_tmp_home -c'ps aux|grep $$' --bash-opts="--rcfile $HOME/xt_tmp_home/.bash_profile"
expr "x$bash_opts" : '.*--rcfile' >/dev/null && home=$HOME || home=$pseudo_home

cp $HOME/.Xauthority "$pseudo_home"
cd "$pseudo_home" >/dev/null
env -i SHELL=$SHELL PATH=/usr/bin:/bin LOGNAME=$USER USER=$USER \
 DISPLAY=$DISPLAY \
 REALHOME="$HOME" HISTFILE="$HOME/.bash_history" \
 HISTTIMEFORMAT='%a %m/%d %H:%M:%S  ' \
 HOME=$PWD CMD_STR="$CMD_STR" $term $term_geom $term_opts $bash $bash_opts &
