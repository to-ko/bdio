#!/bin/bash

cat test.dat justaheader.dat > tmp
cat tmp justaheader.dat > concat.dat
cat concat.dat test2.dat > tmp
cat tmp justaheader.dat > concat.dat

rm tmp
cp concat.dat ../tools/concat.dat 
