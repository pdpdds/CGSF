#!/bin/csh
echo "These tests may take a while.  Be patient."
if (-e BTree.out) rm BTree.out
echo "Running BTree test..."
../BTree/BTree < words.sql > BTree.out
diff BTree.out BTree.std
rm BTree.out
rm wordtmp
if (-e RTree.out) rm RTree.out
echo "Running RTree test..."
../RTree/RTree < boxes.sql > RTree.out
diff RTree.out RTree.std
rm RTree.out
rm boxtmp
if (-e RSTree.out) rm RSTree.out
echo "Running RSTree test..."
../RSTree/RSTree < boxes.sql > RSTree.out
sed s/RST/RT/g RSTree.out | sed "s/R.-Tree/R-Tree/g" | diff - RTree.std
rm RSTree.out
rm boxtmp
