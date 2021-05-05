Jelen programot a WHERE szűrős feltétel idejének vizsgálatára használtam.

Kód fordítása: 
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL -lmysqlcppconn

A mérések repprodukálásához:

az examp.cl fájlban taéálható
    if( i< *limit &&  T1[i].c3==1 && T1[i].c4>5001 )
feltételt kell úgy változtatni, hogy megfeleljen az adott lekérdezésnek.

Futtatás:
./example.out


