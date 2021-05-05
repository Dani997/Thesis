Jelen program a globális méretből adódó sebességek grafikonjaihoz szükséges adatokat biztosítja.

fordítás:
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL -lmysqlcppconn

A program az eredményeit a 
measurements.csv fájlba küldi.

A látott mérések megismétléséhet a kernel kódban található feltételt kell változtatni
a megfelelő elemszám eléréséhez.
Illetve a head.hpp és az examp.cl ben található 
 // int calc1;
 // int calc2;
két sor kivehető kommentből
illetve a kernel feltételben igaz ágában található:
       //ANS[ loc ].calc1 =  T1[i].c3 * T1[i].c4;
       // ANS[ loc ].calc2 = T1[i].c2 * T1[i].c3;
sorok is.

Ezzel átállítható a program a nagyobb erődorrás igényű mérésre.
