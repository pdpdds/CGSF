SET BISON_SIMPLE=\bin\bison.simple
SET BISON_HAIRY=\bin\bison.hairy

cd RTree
del gram.tab.cpp
bison -dv gram.y
ren gram_tab.c gram.tab.cpp
ren gram_tab.h gram.tab.h
flex -It scan.l > scan.cpp

cd ..\RSTree
del gram.tab.cpp
bison -dv gram.y
ren gram_tab.c gram.tab.cpp
ren gram_tab.h gram.tab.h
flex -It scan.l > scan.cpp

cd ..\BTree
del gram.tab.cpp
bison -dv gram.y
ren gram_tab.c gram.tab.cpp
ren gram_tab.h gram.tab.h
flex -It scan.l > scan.cpp

cd ..

