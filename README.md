Knight vs Monsters - 2D Brawler Engine

Kompletny, autorski silnik gry akcji 2D (typu beat 'em up) napisany w języku C++20 z wykorzystaniem biblioteki SDL2 do renderowania grafiki i obsługi zdarzeń. Projekt prezentuje zaawansowane techniki programowania, zarządzania pamięcią oraz strukturę opartą na maszynie stanów.

Technologie: C++20, SDL2, CMake, Make.

Kluczowe rozwiązania architektoniczne:
* System walki i Input Buffering: Zaprojektowano system buforowania wejścia (rejestracja zdarzeń klawiatury wraz ze stemplami czasowymi SDL_GetTicks), pozwalający na detekcję złożonych kombinacji klawiszy (Combos) w czasie rzeczywistym.

* Fizyka i przestrzeń 2.5D: Implementacja niestandardowej osi Z w środowisku dwuwymiarowym, umożliwiająca obsługę grawitacji, skoków oraz detekcję kolizji ataków w powietrzu.

* Data-Driven Design: Parametry gry (układ poziomów, statystyki, definicje ataków i kombinacji) wczytywane są dynamicznie z zewnętrznych plików tekstowych (.txt), co oddziela logikę kodu od danych (Asset parsing).

* Zarządzanie Pamięcią i Narzędzia: Ręczne, bezpieczne zarządzanie cyklem życia obiektów (alokacja map i przeciwników) oraz wsparcie dla systemów budowania projektów wieloplikowych (CMake).

* Sztuczna Inteligencja: Przeciwnicy wyposażeni w maszyny stanów (State Machines) zachowań (np. śledzenie gracza z zachowaniem dystansu, system szarży z czasem odnowienia).
