INC=/usr/local/ssl/include/
LIB=/usr/local/ssl/lib/

A:
	gcc -I $(INC) -L $(LIB) -o A.exe A.cpp -lcrypto -ldl
KM:
	g++ -I $(INC) -L $(LIB) -o KM.exe KM.cpp -lcrypto -ldl
B:
	gcc -I $(INC) -L $(LIB) -o B.exe B.cpp -lcrypto -ldl
exA:
	./A.exe
exKM:
	./KM.exe
exB:
	./B.exe


output: MyShell.c
	gcc MyShell.c -o MyShell.exe
	gcc MyStat.c -o MyStat.exe
	gcc MyFind.c -o MyFind.exe
execute: MyShell.exe
	./MyShell.exe
