#!/bin/sh

rm -f *.odb
./localclitest
./testalter
./testconcur
./testddl
./subsql testddl.sql
./testddl
./testfuzzy
./testidx
./testiref
./testleak
./testfrag
./testperf
./testindex
./testraw
./testsync
./testspat
./testarridx
./testudt
./testwcs
./testtimeseries
./testpar
./testconc update & ./testconc inspect 2


