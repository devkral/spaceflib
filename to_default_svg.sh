#! /usr/bin/bash

basefile="$0"
basedir=`dirname "$basefile"`

srcdir="$basedir/docs"
outdir="$basedir/build/tmp_for_svgs"


if [ "$#" -lt 1 ]; then
  echo "$0 <target name> [params for cpu analyse]"
  exit 1
fi
name="$1"
shift 1

if [ ! -e "$srcdir/$name".*.heap ]; then
  echo "$srcdir/$name.heap does not exist"
  exit 1
fi

if [ ! -e "$srcdir/$name.prof" ]; then
  echo "$srcdir/$name.prof does not exist"
  exit 1
fi


if [ "$#" -lt 1 ]; then
  # by default skip nothing
  params_cpu="--maxdegree=100 --nodecount=400 --nodefraction=0 --edgefraction=0 --focus 'main' --ignore 'Catch'"
  skip_heap=0
else
  params_cpu="$@"
  skip_heap=1
fi


mkdir -p "$outdir"
if [ "$skip_heap" -eq 0 ]; then
  # should include all data
  command="pprof --nodecount=400 --maxdegree=100 --nodefraction=0 --edgefraction=0 --focus 'main' --ignore 'Catch' --svg --alloc_space --show_bytes '$basedir/build/test/print_graph' '$srcdir/$name'.*.heap > '$outdir/${name}_heap.svg'"
  echo "$command"
  eval "$command" &
else
  echo "skip heap"
fi
# configurable
command="pprof $params_cpu --svg '$basedir/build/test/print_graph' '$srcdir/$name.prof' > '$outdir/${name}_cpu.svg'"
echo "$command"
eval "$command" &
wait
