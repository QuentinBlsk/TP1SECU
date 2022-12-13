## Devoir de sécurité numéro 1 
## Nathan Cerveau - Quentin Blasiak

### CVE 2017-16994

### Étude bibliographique et compréhension du mécanisme 

#### 1. Type de compromission

La faille étudiée ici s'applique au niveau du kernel Linux, pour toutes les versions précédant la 4.14.2, version à laquelle elle a été patchée. Elle s'applique à la fonction *mincore* du fichier **mm/pagewalk.c**.

Le type de compromission étudié ici est une potentielle divulgation de l'information. 

#### 2. Description de la vulnérabilité et des façons de l'exploiter

##### 2.1 Description des fonctions appelées

La fonction ``` do_mincore() ``` est une fonction qui prend en entrée une adresse mémoire de départ ``` addr ```, une taille de mémoire à explorer ```length ``` et une référence à un tableau nommé ```vec```. Cette fonction prend aussi en compte la taille **PAGE_SIZE** d'une page dans la mémoire de la machine sur laquelle elle est exécutée (nous vous laissons vous référer au cours de SEPC de l'année dernière pour plus d'informations sur la pagination de la mémoire).

Comme le parcours de la mémoire se fait page par page, le parcours doit commencer en début de page (i.e. ```addr``` doit référencer une adresse qui est un multiple de **PAGE_SIZE**), et s'effectuer sur un nombre entier de pages (l'argument ```length``` est automatiquement arrondi au multiple de **PAGE_SIZE** supérieur). ```vec ``` doit enfin être un tableau contenant au moins *(length + **PAGE_SIZE** - 1)/**PAGE_SIZE*** bytes.

La fonction a pour but de vérifier si la zone mémoire parcourue est utilisée en RAM ou non. Pour chaque page, si elle se situe effectivement dans la ram, le bit de poids faible du byte correspondant du tableau (le byte d'indice 0 correspond à la première parcourue, etc etc) est fixé à 1. La valeur de retour est 0 si l'exécution de la fonction n'a pas engendré d'erreur et -1 sinon. 

Pour effectuer son parcours de zone mémoire, la fonction ```do_mincore()``` fait appel à la fonction ``` walk_page_range()``` qui prend en argument, entre autres, une *struct mm_walk* qui est une struct de callbacks, qui sont appelées pour lire la page en cours de lecture et, dans notre cas, déterminer si elle est dans la RAM ou non, et une taille de mémoire ( convertie en pages) à parcourir. 

Enfin, pour comprendre l'exploitation de la faille, on explique rapidement ce qu'est une page de type HugeTLB en mémoire : un TLB est un cache présent en mémoire pour traduire des adresses virtuelles en adresses physiques. 
Les HugePages permettent de définir une zone RAM dans laquelle les pages ne sont pas gérées avec une taille classique mais une taille bien supérieure (de l'ordre du kilo et du méga respectivement). Les pages HugeTLB sont ainsi utilisées pour optimiser la gestion du TLB.

##### 2.2 Description et exploitation de la faille

Dans les versions du Kernel Linux précédant la 4.14.2, la fonction ```do_mincore()``` considère qu'elle recevra toujours un callback pour chaque page parcourue dans la range de ```walk_page_range()```. Cependant, si ```walk_page_range``` est appellée sur une page de type HugeTLB, les callbacks de la struct passés ne s'appliquent qu'à la zone mémoire effectivement utilisée par le TLB. En l'absence de callback pour une page, l'appel à ```do_mincore()``` va copier de la mémoire non utilisée dans la fonction dans l'allocateur de pages pour l'utilisateur.

Ainsi, en exploitant cette faille, l'utilisateur peut accéder à des espaces mémoire de la machine qui ne devraient pas lui être accessibles.

#### 3. Cette faille concerne-t-elle des machines clients ou des machines serveurs ? 

Comme ici on parle d'une faille du kernel Linux, il s'agit évidemment d'une faille concernant des machines clients. 

#### 4. Un exemple d'architecture d'un SI pouvant exploiter ces failles 

TODO 

#### 5.Comment limiter voire empêcher l'exploitation de cette faille ?

La correction apportée à cette faille est assez simple : le souci venant du fait que la fonction ```walk_page_range``` ne renvoie pas un callback pour chaque page, il suffit de raise une erreur lorsque la page parcourue n'a aucun callback associé. Ainsi, on ne pourra parcourir de la mémoire ayant une callback adaptée pour son parcours. 

#### 6. Quelles sont les bonnes pratiques pouvant limiter cette menace ? 

Cette faille est un cas très particulier d'utilisation du Kernel Linux. Cependant, on peut rappeler quelques règles importantes sur la gestion des accès mémoire.

Le *Guide des Règles de Programmation pour le Développement Sécurisé [...] en Langage C* de l'ANSSI précise dans son article 15.3 que lors du développement de logiciels en C, "Le succès d’une allocation mémoire doit toujours être vérifié."
Si cela s'applique ici à l'allocation de mémoire et donc l'utilisation de la fonction ```malloc()```, on doit quand même vérifier lors du parcours de mémoire interne que l'on engendre pas d'erreur lors du parcours de cette mémoire, et qu'on ne va pas ainsi allouer des espaces mémoire non prévus à cet effet.

#### 7. Comment les développeurs auraient-ils pu éviter l'apparition d'une telle faille ? 

Du débogage du code C à l'aide de GDB permet de voir quels sont les zones mémoire allouées et libérées (il ne faut pas utiliser ```realloc()```, c'est très mal !), et donc de découvrir et prévenir d'éventuelles fuites mémoire. 

#### 8. PSSI

TODO