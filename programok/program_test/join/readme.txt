Jelen programot a JOIN -os SQL lekérdezés idejének vizsgálatára használtam.

Kód fordítása: 
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL -lmysqlcppconn

A mérések repprodukálásához:

az examp.cl fájlban található
kernel kódokat kell módosítani a megfelelő képpen
az első kernel kód az M tábla szűrésére
a második az S tábla szűrésére
és a harmadik a két tábla kapcsolására szolgál


Futtatás:
./example.out



