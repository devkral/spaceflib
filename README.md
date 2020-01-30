
Setup:
======
git submodule init      # prepare subrepositories

git submodule update    # fill subrepositories

cd ./build

cmake ..


Usage:
======
Switch to build directory

./test/print_graph -g=-1 # use inbuild graph to analyse

./test/print_graph -g=<graphid> # connect to server (default localhost) on port and analyse remote graph

./test/print_graph -g=<graphid> -f true # show only cutvertices and connected components

./test/test_spacef -g=<graphid> # tests library against graph (can use external server)


Documentation:
--------------
see build/docs/index.html
