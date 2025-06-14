# IB109
IB109 mini projects 1 (lock free vs posix queue) and 2 (MPI)


### Mini-projekt 01

Vaším cílem v rámci tohoto projektu je naimplementovat s využitím metod Lock-Free programování thread-safe datovou strukturu fronty (požadované metody jsou enqueue -- zařaď prvek do fronty, dequeue -- vyber prvek z fronty a isEmpty --- otestuj, zda je fronta prázdná) a vystavit ji zátěži několika souběžně běžících vláken, které vkládají a vybírají z fronty. Následně upravte svoji implementaci tak, aby místo lock-free přístupu používala pouze POSIXové rozhraní a test opakujte. Porovnejte výsledky vašeho měření.

Aby vaše řešení bylo uznáno, musí splnit všechny následující podmínky.

umístěno v jednom adresáři, odevzdaný jako tar.gz soubor
provedením příkazu make v daném adresáři na stroji aisa se přeloží a spustí Váš kód
Váš kód nesmí běžet déle jak 3 vteřiny
aplikace vypíše na konzolu právě 2 řádky
na 1. prvním řádku Vaše učo
na 2. druhém řádku jedno celé číslo, které vyjadřuje kolik procent času POSIXového řešení zabralo Lock-Free řešení (tj. bude-li Lock-Free přístup o pětinu rychlejší, vypíše se 80, pokud o pětinu pomalejší, vypíše se 120)


### Mini-projekt 02

Vaším cílem v tomto mini-projektu je naimplementovat MPI aplikaci, která po spuštění na n-procesech pomocí metod házení mince určí mezi participujícími procesy jednoho vládce. Informaci, kdo je zvoleným vládcem, musí  po jeho zvolení znát všichni účastníci výpočtu, což doloží tím, že vypíší na konzolu text "Sloužím ti, můj vládče, slunce naše jasné." a doplní tento text identifikátorem procesu vládce v komunikátoru MPI_COMM_WORLD.

Protokol házení mincí je vícekolový protokol, přičemž princip volby je následující. V každém kole každý, kdo je ještě ve hře o pozici vládce, hodí mincí. Pokud padla alespoň jednou v celém distribuovaném výpočtu v tomto kole panna (řekněmě hodnota 1) tak všichni, kteří jsou ještě ve hře a padl jim v tomto kole orel (hodnota 0), vypadávají ze hry. Kola se opakují, dokud není ve hře právě jeden poslední proces.

Aby vaše řešení bylo uznáno, musí splnit všechny následující podmínky.

umístěno v jednom adresáři, odevzdaný jako tar.gz soubor
provedením příkazu make v daném adresáři na stroji nymfe50 se přeloží a spustí Váš kód na 6 processech
(můžete předpokládat, že mpirun a mpicc jsou dostupné)
aplikace jako první vypíše Vaše učo (právě jednou) a pak budou následovat výpisy požadované výše
