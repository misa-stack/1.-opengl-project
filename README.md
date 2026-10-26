# Jak funguje tento OpenGL projekt

Program vykresluje obrys 3D domu. Můžeš ho otáčet, posouvat a měnit jeho velikost.
Hrany za domem se vynechávají podle toho, odkud se na dům díváš.
Veškerá geometrie se kreslí čárami přes funkci `cara()`.

Tento dokument vysvětluje aktuální [src/main.cpp](src/main.cpp) a vlastní kameru
v [src_utils/camera.cpp](src_utils/camera.cpp). Pohled, perspektivu a ořezávání
čar počítá náš C++ kód. Příklady můžeš ověřovat změnou hodnot v programu.

## 1. Soubory a spuštění

| Soubor nebo adresář | K čemu slouží |
| --- | --- |
| `src/main.cpp` | Okno, vstup z klávesnice, transformace, viditelnost a kreslení domu. |
| `src_utils/camera.h` | Vlastní typy `Vec3` a `Camera`, bez grafických závislostí. |
| `src_utils/camera.cpp` | Pohyb, rozhlížení, převod do pohledu, ořezávání a perspektiva. |
| `tests/camera_test.cpp` | Číselné testy kamery, které nepotřebují grafické okno. |
| `src_utils/utils.*` | Starší pomocník `makeFrustum()`; současná kamera ho nepoužívá. |
| `src_utils/stb_image.*` | Přibalený kód pro načítání obrázků; současný dům ho nevyužívá. |
| `include/`, `lib/`, `lib-mingw32/` | Přibalené závislosti a knihovny pro Windows. |
| `data/` | Obrázky pro případné textury; nyní se na dům nenanáší. |
| `CMakeLists.txt` | Určuje zdrojové soubory, C++17 a připojené knihovny. |
| `Makefile` | Na Linuxu spustí CMake a zkopíruje program do kořene projektu. |
| `build/` | Vygenerované soubory a přeložený program. |

Z kořenového adresáře projektu na Linuxu:

```sh
make
./opengl
```

Případně přímo přes CMake:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
(cd build && ./opengl)
```

Potřebuješ překladač podporující C++17, CMake alespoň 3.24, OpenGL a vývojovou
knihovnu GLEW. Pokud systém nemá GLFW, CMake stáhne a sestaví GLFW 3.4; první
konfigurace pak potřebuje internet a platformní vývojové závislosti.
V Qt Creatoru otevři `CMakeLists.txt`.

## 2. Aktuální ovládání

| Klávesa | Výsledek při držení |
| --- | --- |
| Šipka nahoru / dolů | Zvýšení / snížení úhlu kolem X. |
| Šipka doprava / doleva | Zvýšení / snížení úhlu kolem Y. |
| E / Q | Zvýšení / snížení úhlu kolem Z. |
| W / S | Pohyb kamery dopředu / dozadu ve směru pohledu. |
| D / A | Pohyb kamery doprava / doleva vzhledem k pohledu. |
| Mezerník / Ctrl | Pohyb kamery nahoru / dolů podél světové osy Y. |
| Pravé tlačítko myši + pohyb myši | Rozhlížení kamerou. Po uvolnění se vrátí kurzor. |
| Tab | Přepnutí WASD mezi ovládáním kamery a domu. |
| 1 | Zvětšování: `scale += 0.01` každý snímek. |
| 2 | Zmenšování: `scale -= 0.01` každý snímek. |
| R | Reset domu i kamery, včetně směru pohledu. |
| Escape | Ukončení programu. |

Více kláves můžeš držet současně. Protilehlé směry rotace nebo pohybu se ruší.
Při současném držení 1 a 2 vyhraje 1, protože druhá větev používá `else if`.
Reset se provádí až po ostatních změnách, takže je v daném snímku přepíše.
Výchozí režim WASD ovládá kameru. Po Tab se D/A používá pro světovou osu X domu
a W/S pro jeho světovou osu Y. Zvolený režim se resetem R nemění. Rotace a
škálování domu fungují v obou režimech; myš také vždy otáčí kamerou.

Rotace a pohyb používají uplynulý čas. **Škálování ho zatím nepoužívá**, a proto
je jeho rychlost závislá na FPS. Klávesy 1 a 2 v této verzi nenastavují pevné
předvolby velikosti.

## 3. Knihovny a hlavičky

```cpp
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "camera.h"
```

- `iostream` poskytuje `cout`, `cerr` a `endl` pro výpisy do terminálu.
- **OpenGL** zpracovává grafické příkazy a vykresluje do okna.
- **GLEW** načítá dostupné funkce OpenGL po vytvoření grafického kontextu.
- **GLFW** vytváří okno a grafický kontext a zpracovává klávesnici, myš a čas.
- `math.h` poskytuje například `sin()`, `cos()`, `tan()`, `atan()` a `fmod()`.
- `camera.h` zpřístupňuje naši kameru a typ pro 3D souřadnice.

GLM je v projektu přibalená matematická knihovna, ale současné transformace
počítáme ručně a hlavní soubor ji přímo nepoužívá. `camera.cpp` používá jen
standardní knihovnu C++; nevolá OpenGL, GLFW ani GLM.

`using namespace std;` dovoluje psát `cout` místo `std::cout`.
Samotné `#include` vloží deklarace z hlavičky; sestavení ještě musí připojit
potřebné knihovny. To zajišťuje CMake pomocí `target_link_libraries()`.

## 4. Vytvoření okna a callbacky

Globální proměnné:

```cpp
GLFWwindow * glfw_window = NULL;
bool keep_running = true;
```

`glfw_window` je ukazatel na okno spravované GLFW. `NULL` znamená, že zatím na
žádné okno neukazuje. `keep_running` určuje, jestli má pokračovat hlavní smyčka.

`main()` volá:

```cpp
createWindow(1280, 720, "OpenGL", true, false)
```

Argumenty jsou šířka, výška, titulek, zapnutí VSync a požadavek na core profil.
Poslední argument je zde `false`: nepoužije se větev požadující OpenGL 4.2 core.
Současné kreslení přes `glBegin()` potřebuje kompatibilní starší rozhraní.

Funkce `createWindow()` postupně:

1. Ověří, že okno už nebylo vytvořeno, a inicializuje GLFW.
2. Požádá o 24bitový depth buffer a vytvoří okno.
3. Zaregistruje callbacky, tedy funkce volané při příslušné události.
4. Nastaví kontext okna jako aktuální pro grafické příkazy.
5. Pomocí `glfwSwapInterval(1)` zapne synchronizaci výměny snímků.
6. Vypíše informace o grafice a inicializuje GLEW.

`glfw_error_callback()` vypisuje chyby. `glfw_key_callback()` při stisku Escape
nastaví `keep_running = false`. Parametr `key` říká kterou klávesu, `action`
rozlišuje stisk, uvolnění nebo opakování; `scancode` a `mods` zde nevyužíváme.
Při stisku Tab callback přepne cíl ovládání WASD. Callback tlačítka myši při
držení pravého tlačítka zapne relativní rozhlížení a skryje kurzor pomocí GLFW.
Předtím si uloží výchozí pozici, aby první pohyb nezpůsobil skok pohledu.
Callback pohybu předává rozdíl souřadnic metodě `camera.look()`.
Po uvolnění pravého tlačítka nebo ztrátě zaměření okna se kurzor uvolní.

Podmíněná část pro Wayland řeší konkrétní výsledek GLEW: chybějící GLX displej
na platformě, která používá EGL. Ostatní chyby GLEW vedou k uvolnění okna a
neúspěšnému návratu z funkce.

## 5. Souřadnice domu

Každý bod má tři souřadnice:

```text
x = šířka, kladný směr doprava
y = výška, kladný směr nahoru
z = hloubka, kladný směr směrem k počáteční kameře
```

Původní model má tyto rozměry:

| Část | Souřadnice |
| --- | --- |
| Levá / pravá stěna | `x = -250` / `x = 250` |
| Podlaha | `y = -200` |
| Horní okraj obdélníkových stěn | `y = 100` |
| Hřeben střechy | `x = 0`, `y = 250` |
| Přední / zadní strana | `z = 175` / `z = -175` |

Jednotky jsou naše vlastní světové jednotky, nikoli přímo pixely nebo metry.
Počátek `(0, 0, 0)` leží uvnitř domu a je středem jeho rotace a škálování.
Nemusí to být přesný geometrický střed celého modelu.

Například:

```cpp
cara(-250, -200, 175, 250, -200, 175);
```

spojuje dolní levý a dolní pravý roh přední stěny. První tři čísla popisují
počáteční bod a druhá trojice koncový bod.

## 6. Funkce `bod()`: od modelu ke světu

`bod()` obdrží bod v původních souřadnicích modelu a provede:

```text
původní bod
  → změna velikosti
  → rotace kolem X
  → rotace kolem Y
  → rotace kolem Z
  → posun ve světě
  → vrácení světového bodu jako Vec3
```

Parametry `x`, `y`, `z` se předávají hodnotou. Jejich změna uvnitř funkce
nepřepisuje souřadnice zapsané v jednotlivých voláních `cara()`.
Každý snímek tak znovu vychází z původního tvaru domu.

### Změna velikosti

```cpp
x *= scale;
y *= scale;
z *= scale;
```

`x *= scale` znamená `x = x * scale`.
Při `scale = 0.5` se modelové rozměry zmenší na polovinu.
Při `scale = 2.0` se zdvojnásobí. Kvůli perspektivě nemusí být výsledná šířka
na obrazovce přesně dvojnásobná.

Měřítko se aplikuje kolem počátku modelu. Následný posun domu se jím nenásobí.

### Úhly a radiány

`sin()` a `cos()` očekávají úhel v radiánech:

```text
180 stupňů = pi radiánů
90 stupňů = pi / 2 radiánů
60 stupňů = 60 * pi / 180 radiánů
```

Úhly `angleX`, `angleY`, `angleZ` jsou v radiánech. Znaménka rotací odpovídají
pravidlu pravé ruky: palec ukazuje ve směru kladné osy a prsty ve směru kladné
rotace. Výsledek na obrazovce záleží i na aktuálním natočení modelu.

### Rotace kolem X

Osa X zůstává stejná, mění se Y a Z:

```text
y1 = y * cos(angleX) - z * sin(angleX)
z1 = y * sin(angleX) + z * cos(angleX)
```

Při otočení o kladných 90° se bod `(0, 100, 0)` dostane do `(0, 0, 100)`.
V kódu jsou výsledky pojmenované `yAfterX` a `zAfterX`.

### Rotace kolem Y

Vycházíme už z výsledku první rotace:

```text
x2 = x * cos(angleY) + z1 * sin(angleY)
z2 = -x * sin(angleY) + z1 * cos(angleY)
```

Y se v tomto kroku nemění. Samostatná kladná rotace o 90° kolem Y převede
bod `(100, 0, 0)` na `(0, 0, -100)`.

### Rotace kolem Z

```text
x3 = x2 * cos(angleZ) - y1 * sin(angleZ)
y3 = x2 * sin(angleZ) + y1 * cos(angleZ)
```

Z zůstane `z2`. Samostatná kladná rotace o 90° převede `(100, 0, 0)` na
`(0, 100, 0)`. Rotace obecně nelze libovolně přehazovat: X → Y → Z může dát
jiný výsledek než Z → Y → X.

### Posun

```cpp
const double translatedX = rotatedX + positionX;
const double translatedY = rotatedY + positionY;
```

Posun se přičítá až po rotaci. Dům se proto otáčí kolem svého modelového počátku
na nové pozici. Kdyby se nejdřív posunul a pak otočil, otáčel by se i vektor
posunu a dům by obíhal kolem světového počátku.

Nakonec se celý 3D bod vrátí volající funkci:

```cpp
return {translatedX, translatedY, zAfterY};
```

`Vec3` je jednoduchý vlastní `struct` se třemi hodnotami `double`: `x`, `y`, `z`.
Funkce `cara()` tak dostane oba konce úsečky ve světových souřadnicích a teprve
potom je předá vlastní kameře.

## 7. Vlastní kamera, perspektiva a ořezávání

Kamera má vlastní pozici `position`, vodorovný úhel `yaw` a svislý úhel `pitch`.
Výchozí pozice je `(0, 0, 500)`, oba úhly jsou nulové a kamera se dívá podél
záporné světové osy Z. Kladný yaw otáčí pohled doprava, kladný pitch nahoru.

### Převod světa do pohledu: `Camera::toView()`

Nejdřív od bodu odečteme pozici kamery:

```text
dx = světovéX - camera.position.x
dy = světovéY - camera.position.y
dz = světovéZ - camera.position.z
```

Z yaw a pitch si sami spočítáme tři kolmé jednotkové směry kamery:

```text
right   = (cos(yaw), 0, sin(yaw))
up      = (-sin(yaw)*sin(pitch), cos(pitch), cos(yaw)*sin(pitch))
forward = (sin(yaw)*cos(pitch), sin(pitch), -cos(yaw)*cos(pitch))
```

Každou souřadnici pohledu získáme skalárním součinem rozdílu `(dx, dy, dz)`
s odpovídajícím směrem. Skalární součin znamená sečíst součiny složek:

```text
viewX = dx*right.x + dy*right.y + dz*right.z
viewY = dx*up.x + dy*up.y + dz*up.z
depth = dx*forward.x + dy*forward.y + dz*forward.z
```

V našem typu `Vec3` se hloubka uloží do `z`. **Kladná hloubka je před kamerou**.
Při výchozím pohledu proto vyjde `depth = 500 - světovéZ`.
To je jiná souřadná soustava než svět, kde kladné Z míří k výchozí kameře.

### Ořezávání úsečky před projekcí

`Camera::projectLine()` nejprve převede oba konce do pohledu. Potom hledá část
úsečky uvnitř šesti rovin: blízké, vzdálené, levé, pravé, dolní a horní.

Platí:

```text
1 <= depth <= 5000
-3.2*depth <= viewX <= 3.2*depth
-1.8*depth <= viewY <= 1.8*depth
```

Čísla 3.2 a 1.8 jsou poloviny šířky a výšky obrazu dělené ohniskovou
vzdáleností: `640 / 200` a `360 / 200`.
Úsečku zapisujeme jako `A + t*(B-A)`, kde `t` jde od 0 do 1. Pro každou rovinu
omezíme přípustný interval `t`. Pokud leží oba konce za stejnou rovinou,
úsečku rovnou vynecháme. Pokud ji protíná, průsečík má parametr:

```text
t = hodnotaRoviny(A) / (hodnotaRoviny(A) - hodnotaRoviny(B))
```

Kladná hodnota roviny značí vnitřek. Ořezávání se provede **před dělením
hloubkou**, takže můžeme bezpečně zpracovat i úsečku procházející kamerou.

### Ruční perspektivní dělení

Z oříznutých bodů vypočítáme normalizované souřadnice:

```text
normalizedX = viewX * 200 / (depth * 640)
normalizedY = viewY * 200 / (depth * 360)
```

Rozsah od -1 do 1 odpovídá celé šířce nebo výšce viewportu. Střed je nula.
Menší hloubka dává větší obraz stejně velkého předmětu. Výchozí ohnisková
vzdálenost 200 zachovává původní velikost domu na obrazovce.
Také hloubku sami převedeme do rozsahu -1 až 1:

```text
normalizedZ = (far + near)/(far - near) - 2*far*near/((far - near)*depth)
```

Na blízké rovině vyjde -1 a na vzdálené +1. Převod je nelineární, aby hloubka
odpovídala perspektivě také podél vykreslených úseček.

`init()` ponechává projekční i modelview matici jednotkovou. Nepoužívá
`makeFrustum()`, `glFrustum()`, `glTranslated()`, GLM ani `lookAt`.
OpenGL dostává hotové normalizované konce čar a převádí je na obrazové fragmenty.

`glViewport()` ponechává pevnou oblast 1280 × 720 uprostřed framebufferu.
Program tedy při změně velikosti okna scénu automaticky nepřizpůsobuje;
v menším okně může být část obrazu oříznutá.

### Pohyb a rozhlížení

`Camera::move()` složí pohyb z pravého a dopředného směru a světové osy Y.
Výsledný vektor vydělí jeho délkou, takže držení více směrů nezvýší rychlost.
Potom jej násobí rychlostí 200 jednotek za sekundu a časem `dt`.
Protože dopředný směr zahrnuje i pitch, W při pohledu vzhůru také stoupá:
jde o volně létající kameru bez gravitace a kolizí.

`Camera::look()` násobí rozdíl pixelů myši citlivostí 0.0025 radiánu na pixel.
Vodorovný pohyb mění yaw. Svislá souřadnice myši roste dolů, proto se od pitch
odečítá. Pitch je omezen na ±89°, takže se kamera nepřeklopí přes svislý směr.
`Camera::reset()` vrátí pozici `(0, 0, 500)` i oba úhly na výchozí hodnoty.

## 8. Klávesnice a čas `dt`

`glfwPollEvents()` v hlavní smyčce zpracuje události. Potom `render()` kontroluje
stav držených kláves pomocí `glfwGetKey()`.

```cpp
const int directionY = (glfwGetKey(glfw_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    - (glfwGetKey(glfw_window, GLFW_KEY_LEFT) == GLFW_PRESS);
```

Výraz porovnání má hodnotu `true` nebo `false`. Při odečítání se převede na 1
nebo 0:

| Držené klávesy | Výpočet | Směr |
| --- | --- | --- |
| Žádná | `0 - 0` | `0` |
| Doprava | `1 - 0` | `1` |
| Doleva | `0 - 1` | `-1` |
| Obě | `1 - 1` | `0` |

`dt` je doba jednoho snímku v sekundách. Používáme:

```cpp
const double rotationStep = 60.0 * pi / 180.0 * dt;
const double movementStep = 200.0 * dt;
```

Například při `dt = 0.01` se za snímek otočíme o 0,6° a posuneme o 2 jednotky.
Sto takových snímků představuje jednu sekundu: celkem 60° a 200 jednotek.
Pohyb nezrychlí jen tím, že počítač vykreslí více snímků.

`fmod(angle, 2.0 * pi)` omezuje velikost úhlu pomocí zbytku po dělení jednou
otáčkou. Záporný úhel může zůstat záporný; pro `sin()` a `cos()` to nevadí.
Pohyb kamery je normalizovaný, takže diagonála není rychlejší. Ve starším
režimu pohybu domu se každá osa stále aktualizuje zvlášť, takže je jeho
diagonální pohyb rychlejší než pohyb pouze po jedné ose.

## 9. Proč nejsou vidět zadní hrany

Současný dům je **konvexní**, tedy nemá dovnitř zasahující prohlubně. Při pohledu
zvenku lze viditelnost jeho povrchových hran určit podle sousedních stěn:
hranu nakreslíme, když se ke kameře obrací alespoň jedna ze stěn, které sdílí.
Čáry uvnitř jedné stěny, například okna, zobrazíme jen při viditelnosti té stěny.

Stěny v tomto výpočtu představují roviny a podmínky. **Žádné vyplněné stěny ani
trojúhelníky se neposílají ke kreslení.**

### Proč počítáme polohu kamery vzhledem k modelu

Roviny domu mají jednoduché souřadnice v původním modelu: například přední
stěna je vždy `z = 175`. Po otočení domu už tato rovnice ve světových
souřadnicích neplatí.

Místo otáčení všech rovnic převedeme kameru do původních souřadnic domu.
To vyjadřují proměnné `cameraX`, `cameraY`, `cameraZ`. Jsou to pomocné
souřadnice stejné pohyblivé kamery, určené pro výpočet viditelnosti.

Pro bod domu používáme:

```text
škálování → X → Y → Z → posun
```

Pro opačný převod kamery musíme kroky obrátit:

```text
odečtení posunu → opačná Z → opačná Y → opačná X → dělení měřítkem
```

Po odečtení posunu domu má kamera souřadnice
`(relativeX, relativeY, camera.position.z)`, kde
`relativeX = camera.position.x - positionX` a
`relativeY = camera.position.y - positionY`.
Opačná rotace kolem Z z nich udělá:

```text
cameraXAfterZ = relativeX * cos(angleZ) + relativeY * sin(angleZ)
cameraYAfterZ = -relativeX * sin(angleZ) + relativeY * cos(angleZ)
```

Opačná rotace kolem Y:

```text
cameraZAfterY = cameraXAfterZ * sin(angleY) + camera.position.z * cos(angleY)
cameraX = (cameraXAfterZ * cos(angleY) - camera.position.z * sin(angleY)) / scale
```

Opačná rotace kolem X a dokončení dělení měřítkem:

```text
cameraY = (cameraYAfterZ * cos(angleX) + cameraZAfterY * sin(angleX)) / scale
cameraZ = (-cameraYAfterZ * sin(angleX) + cameraZAfterY * cos(angleX)) / scale
```

Při výchozím nastavení vyjde `(0, 0, 500)`. Po otočení domu o 180° kolem Y
vyjde přibližně `(0, 0, -500)`, takže vidíme zadní stranu modelu.
Výpočet používá stejnou světovou pozici kamery jako projekce. Yaw a pitch zde
nepotřebujeme: určují výřez pohledu, ale ne to, na které straně stěny stojíme.

### Podmínky viditelnosti stěn

| Proměnná | Podmínka | Význam |
| --- | --- | --- |
| `front` | `cameraZ > 175` | Kamera je před přední stěnou. |
| `back` | `cameraZ < -175` | Kamera je za zadní stěnou. |
| `left` | `cameraX < -250` | Kamera je nalevo od domu. |
| `right` | `cameraX > 250` | Kamera je napravo od domu. |
| `bottom` | `cameraY < -200` | Kamera je pod podlahou. |
| `roofLeft` | `cameraY - 0.6 * cameraX > 250` | Kamera je nad vnější stranou levé střechy. |
| `roofRight` | `cameraY + 0.6 * cameraX > 250` | Kamera je nad vnější stranou pravé střechy. |

Levá plocha střechy prochází v řezu body `(-250, 100)` a `(0, 250)`.
Její sklon je `(250 - 100) / (0 - (-250)) = 0.6`, tedy:

```text
y = 0.6 * x + 250
```

Nad touto rovinou platí `y - 0.6*x > 250`. Pravá strana má opačný sklon.

### Jak z podmínek vznikne viditelná čára

```cpp
if (front || right) cara(250, -200, 175, 250, 100, 175);
```

Tato svislá hrana leží mezi přední a pravou stěnou. `||` znamená logické
„nebo“, takže stačí vidět jednu z nich.

Hřeben střechy sousedí s oběma střešními plochami:

```cpp
if (roofLeft || roofRight) cara(0, 250, 175, 0, 250, -175);
```

Dveře a okna patří jen přední stěně, proto jsou jejich čáry uvnitř `if (front)`.
Jsou to nakreslené obdélníky na stěně, nikoli skutečné otvory do interiéru.

Pokud kamera nevychází ven za žádnou z rovin, kód ji považuje za umístěnou
uvnitř domu nebo na jeho hranici. Potom dovolí všechny stěny a vlastní kamera
ořízne čáry mimo pohledový objem.

Tento postup využívá konkrétní konvexní tvar domu. U domu ve tvaru L nebo při
vzájemném zakrývání několika objektů by samotné tyto podmínky nestačily:
bylo by nutné řešit i částečné zakrytí jednotlivých hran.

## 10. `cara()`, `GL_LINES` a depth buffer

```cpp
void cara(int x1, int y1, int z1, int x2, int y2, int z2)
{
    Vec3 first, second;
    if (camera.projectLine(bod(x1, y1, z1), bod(x2, y2, z2), first, second))
    {
        glVertex3d(first.x, first.y, first.z);
        glVertex3d(second.x, second.y, second.z);
    }
}
```

Funkce nejprve získá dva světové body, pak je vlastní kamerou ořízne a promítne.
Pokud je úsečka celá mimo pohled, nic nevykreslí. Jinak předá oba normalizované
konce OpenGL. V aktuálním rozhraní musí být volána uvnitř:

```cpp
glColor3ub(0, 0, 0);
glBegin(GL_LINES);
// Jednotlivá volání cara(...).
glEnd();
```

`GL_LINES` z každé dvojice vrcholů vytvoří samostatnou úsečku.
`glColor3ub()` přijímá tři barevné složky v rozsahu 0–255; tři nuly znamenají
černou. Další `glBegin()` nelze vnořit před odpovídající `glEnd()`.
Nastavení viewportu a podobných stavů se proto provádí mimo tento blok.

V `init()` je také zapnutý `GL_DEPTH_TEST` s funkcí `GL_LEQUAL`.
Zjednodušeně: kreslený fragment projde, pokud jeho hloubka není větší než
uložená hloubka na stejném pixelu. Pomáhá to například při překrývání čar.

**Depth buffer sám v této verzi nezakrývá celou plochu stěn.** Do jejich
vnitřku se nic nekreslí. Zadní hrany domu vynechává výše popsaný výpočet
viditelnosti ještě před voláním `cara()`.

## 11. Hlavní smyčka v `main()`

Po vytvoření okna a zavolání `init()` se opakují tyto kroky:

1. `glfwPollEvents()` zpracuje vstup a události okna.
2. `glClear()` vymaže obraz a staré hloubkové hodnoty.
3. `render(delta)` aktualizuje transformace a vykreslí viditelné hrany.
4. `glfwSwapBuffers()` zobrazí připravený snímek výměnou obrazových bufferů.
5. `glfwGetTime()` umožní změřit uplynulý čas a aktualizovat statistiky.

`delta` při prvním snímku obsahuje nulu. V dalších obsahuje čas naměřený
v předchozí iteraci. Držení kláves tak mění stav postupně mezi snímky.

Podmínka smyčky kontroluje `keep_running` i `glfwWindowShouldClose()`.
Program skončí po Escape nebo požadavku na zavření okna.
Na konci `glfwDestroyWindow()` zničí okno a `glfwTerminate()` uvolní GLFW.

### FPS a časování

- `ticks` počítá vykreslené snímky v měřeném úseku.
- `total_time` sčítá jejich naměřené časy.
- `max_delta` ukládá nejdelší čas jednoho snímku v tomto úseku.
- `ticks / total_time` určuje průměrné FPS.

Přibližně po třech sekundách se statistiky vypíšou a počítadla vynulují.

## 12. Drobnosti a omezení současné verze

- **Výpis milisekund:** kód násobí sekundy číslem `100`, ale pro milisekundy
  by měl používat `1000`. FPS se počítá samostatně a tento překlep ho nemění.
- **Barva pozadí:** hlavní smyčka přepisuje nastavení z `init()` voláním
  `glClearColor(255,255,255,0)`. Barevné složky nad 1 se při mazání omezí na 1,
  proto je pozadí bílé. Obvyklý zápis bílé by byl `(1, 1, 1, 1)`.
- **Nulové měřítko:** `render()` přeskočí kreslení, aby se při převodu kamery
  nedělilo nulou.
- **Záporné měřítko:** není zakázané; převrátí model přes počátek.
- **Velikost bez limitu:** škálování může přesunout části modelu za kameru
  nebo přes ořezávací roviny. Náš `Camera::projectLine()` je potom ořízne.
- **Pevný viewport:** menší okno neznamená automatické zmenšení domu.
- **Viditelnost:** roviny odpovídají právě uvedeným rozměrům domu. Při změně
  geometrie je nutné upravit i související podmínky.

Tyto body popisují současný kód; samotný tento dokument je neopravuje.

## 13. Slovníček zápisů v C++

| Zápis | Význam v tomto programu |
| --- | --- |
| `double` | Desetinné číslo pro souřadnice, úhly a čas. |
| `int` | Celé číslo, například směr `-1`, `0`, `1` nebo rozměr okna. |
| `bool` | Logická hodnota `true` / `false`. |
| `void` | Funkce nevrací žádnou hodnotu. |
| `const` | Proměnnou po inicializaci v daném místě nepřepisujeme. |
| `constexpr` | Hodnota může být určená při překladu; zde například `pi`. |
| `==` | Porovnání rovnosti; samotné `=` znamená přiřazení. |
| `+=`, `-=` | Přičtení / odečtení a uložení zpět do proměnné. |
| `!` | Logické „ne“. |
| `&&` | Logické „a“; obě podmínky musí platit. |
| `||` | Logické „nebo“; stačí jedna podmínka. |
| `return` | Ukončení aktuální funkce, případně s návratovou hodnotou. |
| `#if`, `#endif` | Podmíněný překlad, například části specifické pro Wayland. |
| `struct` | Vlastní datový typ, například souřadnice `Vec3` nebo kamera `Camera`. |
| `Vec3 &` | Odkaz na bod; `projectLine()` tak uloží výsledek do předané proměnné. |
| `Camera::move` | Metoda `move()` patřící typu `Camera`. |

## 14. Jak si porozumění ověřit

Matematické testy kamery spustíš bez otevírání grafického okna:

```sh
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Ověřují například směry pohledu, konstantní rychlost, perspektivu a ořezávání
úseček před kamerou i přes všech šest rovin. Nenahrazují vyzkoušení skutečné
myši a okna. Po `make` a `./opengl` zkoušej po jednom:

1. Ve výchozím pohledu vidíš dveře a okna, ale ne zadní hrany domu.
2. Po přibližně půlotáčce kolem Y zmizí dveře i okna a vidíš zadní stranu.
3. Při pohledu šikmo vidíš společnou hranu dvou viditelných ploch.
4. Při pohledu zespodu jsou viditelné obvodové hrany podlahy.
5. Posuň kameru pomocí WASD a Space/Ctrl; správně se mění i viditelnost hran.
6. Podrž pravé tlačítko myši a rozhlédni se. Po uvolnění nebo přepnutí okna
   se musí vrátit volný kurzor.
7. Přepni Tab, posuň dům přes WASD a otoč ho: posun se nemá změnit v obíhání počátku.
8. Změň velikost a stiskni R: vrátí se dům i poloha a směr kamery.
9. Ověř zavření přes Escape i zavírací tlačítko okna.

Vlastní příklady pro procvičení: na papíře otoč bod `(100, 0, 0)` o 90° kolem
každé osy zvlášť, spočítej viditelnost stěn pro kameru `(0, 0, -500)` a odvoď
rovnici střechy po změně její výšky. Potom porovnej předpověď s programem.
