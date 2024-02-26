# Client Web. Comunicaţie cu REST API

Acest proiect a fost implementata in C, folosind biblioteca parson pentru parsarea raspunsurilor primite de la server, precum si scheletul laboratorului 9.
Proiectul are urmatoarea structura:
- **buffer.c** - contine functiile necesare pentru utilizarea bufferelor corespunzatoare
comenzilor;
- **client.c** - este partea cea mai importanta a temei. Aici se afla functiile necesare
pentru implementarea comenzilor bibliotecii. De asemenea, aici se afla si functia
main, care se ocupa de citirea comenzilor de la tastatura si de apelarea functiilor
corespunzatoare;
- **requests.c** - contine functiile necesare pentru implementarea comenzilor clientului
(se ocupa cu generarea mesajelor de tip HTTP pentru a fi trimise la server); 
- Makefile - contine regulile de compilare;
- **parson.c** - bilbioteca JSON folosita pentru parsarea raspunsurilor primite de la
server;
- **helpers.c** - contine functii ajutatoare pentru stabilirea conexiunii dintre client
si server.

In functie de comanda primita de la tastatura, se apeleaza functia corespunzatoare:
- **register** - se apeleaza functia register_account, care verifica daca datele introduse
sunt valide si, in caz afirmativ, trimite un mesaj de tip POST la server, pentru a
inregistra un nou utilizator. Daca deja exista un utilizator cu acelasi nume, se afiseaza
un mesaj de eroare;
- **login** - se apeleaza functia login_account, care verifica daca datele introduse sunt valide,
iar in caz afirmativ, trimite un mesaj de tip POST la server, pentru a autentifica un
utilizator. Daca datele nu sunt valide sau daca utilizatorul este deja autentificat, se
afiseaza un mesaj de eroare;
- **enter_library** - se apeleaza functia enter_library, care trimite un mesaj de tip GET la
server, pentru a intra in biblioteca. Se verifica daca utilizatorul este autentificat. Daca
nu este, se afiseaza un mesaj de eroare;
- **get_books** - se apeleaza functia get_books, care trimite un mesaj de tip GET la server,
pentru a primi lista de carti din biblioteca. Se verifica daca utilizatorul este autentificat
si daca utilizatorul detine accesul la biblioteca. Daca nu, se afiseaza un mesaj de
eroare;
- **get_book** - se apeleaza functia get_book, care trimite un mesaj de tip GET la server,
pentru a primi informatii despre o carte din biblioteca. Se verifica daca utilizatorul este
autentificat si daca utilizatorul detine accesul la biblioteca.
- **add_book** - se apeleaza functia add_book, care trimite un mesaj de tip POST la server,
pentru a adauga o carte in biblioteca. Se verifica daca utilizatorul este autentificat si daca
utilizatorul detine accesul la biblioteca.
- **delete_book** - se apeleaza functia delete_book, care trimite un mesaj de tip DELETE la
server, pentru a sterge o carte din biblioteca. Se verifica daca utilizatorul este autentificat
si daca utilizatorul detine accesul la biblioteca.
- **logout** - se apeleaza functia logout_account, care trimite un mesaj de tip GET la server,
pentru a deloga un utilizator. Se verifica daca utilizatorul este autentificat. Daca nu este,
se afiseaza un mesaj de eroare.
In main se citeste comanda de la tastatura si se apeleaza functia corespunzatoare. Pentru
a evita pierderea datelor datorita timeout-ului de la server, am ales sa deschid o noua conexiune
pentru fiecare comanda.
