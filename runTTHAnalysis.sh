#!bin/bash
#root -b -l '.L getTTH.C++' -e 'getTTH.C("'$1'")'
root -b -l -q 'runTTHAnalysis.C("'$1'")'
rm *dict* *.so
