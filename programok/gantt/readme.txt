Az itt lévő programot a gantt diagram előállítására használtam.
A kód elején megtalálható a fordításhoz szükséges parancs.
g++ -D_GLIBCXX_USE_CXX11_ABI=0 example.cpp -o example.out -lOpenCL -lmysqlcppconn

A mérések repprodukálásához a GLOBAL_SIZE nevű konstas értéket kell a kód elején változtatni.
512 1024 131072
