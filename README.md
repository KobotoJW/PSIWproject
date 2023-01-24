# PSIWproject
Aby uruchomic: ./PSIWfinal usr1.
W pliku config znajduja sie 3 opcje usr1, usr2, usr3.
Format w pliku config to: nazwa : nazwa_fifo (spacje przed i po dwukropku).
Nalezy uruchomic program dwa razy w osobnych terminalach, np. w pierwszym usr1 a w drugim usr2, poniewaz dopiero wtedy stworza sie i otworza do czytania odpowiednie FIFO.
FIFO tworza sie w folderze w ktorym uruchamiamy program.
Program uzywa fork(), proces child slucha i wykonuje komendy w swojej FIFO, proces parent moze pisac do innych.
Po uruchomienu pojawi sie prosba o komende, po zatwierdzeniu enterem pojawi sie prosba o wskazanie do ktorej FIFO
chcemy wyslac komende.
FIFO sa podane w formacie usr1fifo, usr2fifo, usr3fifo.
Mozna pisac do swojego wlasnego FIFO (np. usr1 moze wyslac ls -l do usr1fifo)
Po podaniu 'exit' zamiast komendy program zamknie child i parent procesy i usunie swoje FIFO.
W kodzie uzylem polecenia system(), zamiast exec().

Przykladowe uzycie (w drugim terminalu uruchomiono ./PSIWfinal usr2): 
1. ./PSIWfinal usr1
2. (Pojawia sie prosba o komende)
3. ls -l
4. (Pojawia sie prosba o wskazanie FIFO)
5. usr2fifo
6. (Pojawia sie kolejna prosba o komende)
7. exit
8. (Program sie zamyka)
