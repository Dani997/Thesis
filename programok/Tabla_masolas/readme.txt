A program a MySQL-ből érkezett adatok memóriábamásolásának idejét méri.

A különféle mérésekhez a lekérdezés szövgében kell módosítani a lekért oszlopok nevét
illetve a
	T1[0][i].c1p1 = res->getInt("c1p1");
	T1[0][i].c2 = res->getInt("c2");
	T1[0][i].c3 = res->getInt("c3");
	//T1[0][i].c4 = res->getInt("c4");

sorok megfelelőit kommentelni.
