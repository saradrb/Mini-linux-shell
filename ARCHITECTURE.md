# Architecture

## Diagramme des classes

Un diagramme montrant les différentes relations entre les classes a été crée pour mieux visualiser ce projet.

IMAGE

Lors de son lancement, `slash` ignore `SIGINT` et `SIGTERM`, affiche son prompt et attend une ligne de commande de l'utilisateur. Selon le contenu de cette ligne de commande, `slash` va effectuer plusieurs actions :
- si la ligne contient un joker (`*` ou `**`), alors il va la traiter à l'aide des méthodes contenu dans `wildcard.c`
- si la ligne contient une redirection, alors 
- traiter les caractères spéciaux (tels que `*`, `**`, `|` et toutes les redirections)
- et enfin exécuter la commande

## Détails des algorithmes implémentés

### Commandes internes
- Commande `exit`

La fonction `exit` prend en argument un tableau d'argument et sa taille, et quitte le programme `slash` avec une valeur de retour. Cette valeur de retour est soit celle de l'argument donné, soit celle de la dernière fonction appelé si aucun argument n'a été donné.

Les différentes étapes de la fonction `exit` sont détaillées ci-dessous :
```
Fonction exit (tab, length) :
    vérification des arguments donnés : si plus d'un argument a été donné alors affiche une erreur
    si aucun argument n'a été donné alors
        quitte le programme avec la valeur de retour de la dernière fonction appelée
    sinon
        vérifie si l'argument donné est un nombre inférieur ou égal à la plus grande valeur d'exit possible : si ce n'est pas le cas alors affiche une erreur
        quitte le programme avec la valeur de retour donnée
```

- Commande `pwd`

La fonction `pwd` prend en argument un tableau d'argument et sa taille, affiche le chemin du répertoire courant, et retourne si la fonction s'est exécuté correctement. Les seuls arguments autorisés sont `-P` et `-L`. Dans le cas où une suite d'option `-P` et `-L` est donné, seule la dernière option est considéré.

Les différentes étapes de la fonction `pwd` sont détaillées ci-dessous :
```
Fonction pwd (tab, length) :
    vérification des arguments données : si un argument n'est pas -P ou -L alors affiche une erreur
    si l'option est -P alors
        affiche le chemin interprété de manière physique
    sinon
        affiche le chemin stocké
```

- Commande `cd`

La fonction `cd` prend en argument un tableau d'argument et sa taille, change le répertoire courant et retourne si le changement de répertoire s'est bien effectué. Les seuls arguments autorisés sont `-P`, `-L`, `-`, et un chemin valide. Dans le cas où une suite d'option `-P` et `-L` est donné, seule la dernière option est considéré.

Les différentes étapes de la fonction `cd` sont détaillées ci-dessous :
```
Fonction cd (tab, length) :
    vérification des arguments donnés : si plus d'un argument qui n'est pas -, -P, -L est trouvé alors affiche une erreur

    si aucun chemin n'a été donné alors
        récupère le contenu de la variable environnementale $HOME
    si - a été donné en argument alors
        récupère le chemin du précédent répertoire courant
    si le chemin doit être interprété de manière logique alors
        si le chemin n'est pas un chemin absolu alors
            concatène le chemin du répertoire courant au chemin donné
        si le chemin contient . ou .. alors
            retire . et .. du chemin en respectant l'interprétation

    changement de répertoire dans le chemin récupéré/traité
    si le changement de répertoire ne s'est pas effectué et que le chemin était interprété de manière logique alors
        changement de répertoire dans le chemin non traité
    
    si le changement de répertoire a été fait avec une interprétation physique alors
        stocke le chemin interprété de manière physique
    sinon
        stocke le chemin traité
```

### Gestion des commandes externes
Les commandes externes à `slash` sont exécutées dans un processus fils, à l'aide de la famille de commande `exec`.

Une explication globale de la fonction exécutant les commandes externes est détaillée ci-dessous :
```
Fonction extern_command(cmd, arguments) : 
    crée un fils
    si c'est le fils alors
        restaure la gestion des signaux par défaut
        exécute la commande
    sinon
        attend le fils
        récupère sa valeur de retour
```

### Gestion des jokers
Le programme `slash` permet l'expansion de deux jokers : `*` et `**/`.

Le joker `*` permet de prendre tous les fichiers respectant le motif donné. Pour ce faire, une fonction `expand_star` a été écrite, qui cherche tous les fichiers correspondant au motif et renvoie un tableau contenant toutes les expansions.

Une explication globale de la fonction permettant l'expansion du joker `*` est détaillée ci-dessous :
```
Fonction expand_star (path, length, current_path) :
    si le chemin path à expandre ne contient qu'un seul fichier alors
        cherche toutes les options du répertoire courant current_path qui correspondent au motif
        concatène toutes les options trouvées avec le répertoire courant
        retourne toutes les options concaténées
    sinon (path est un chemin) // path contient un repertoire (A/*B/*.c)
        tant que path contient plus de deux elements (le premier est forcement un repertoire) 
            si l'element contient * 
            cherche toutes les options du répertoire courant qui correspondent au motif
            sinon la seule option est l'element lui meme (repertoire ne contient pas * ex: A/*.c)
            Pour cahque option trouvé, 
                la concatener avec le répertoire courant current_path
                supprimer l'element de path
                faire appel recursive de expand _star pour chercher le path restant dans le nouveau reperoire courant (repetoire courant c'est current path est concatené avec l'option trouvé)
            
```

Le joker `**/` permet quant à lui de prendre tous les chemins physiques ayant comme suffixe ce qu'il se trouve après le joker. L'implémentation de cette méthode, `expand_double_star`, est ainsi similaire à celle de `expand_star`.

Une explication globale de la fonction permettant l'expansion du joker `**/` est détaillée ci-dessous :
```
Fonction expand_double_star (path, length, current_rep) :
    on supprime le joker **/ du path 
    si path contient seulement le joker donc path devient vide apres suppression alors
        renvoie toute l'arborescence du répertoire courant
    sinon
        Parcourir toute  l'arborescence du repertoire_courant et dans chaque repertoire de l'arborescence 
            Appelle expand_sta(path,length,current_rep) pour recuperer toute les options de path à partir de ce ce repertoire
            concatène les options trouvé dans un tableau recursivent
        renvoie toute les options trouvé dans tout les repertoire de l'arborecence initial 
        
```

### Gestion des redirections

Le programme `slash` permet de rediriger l'entrée standard, la sortie standard et la sortie erreur des différentes commandes éxécutées. Pour celà, on teste si la ligne de commande contient une redirection, et si oui on l'effectue.

Une explication globale de la fonction permettant les redirections est détaillée ci-dessous :
```
Fonction redirection (arguments) :
    tant que arguments contient des redirections faire
        récupère le symbole de redirection
        récupère le fichier d'origine/destination 
        tester si le champs de redirection est valide : il exite un champs apres le symbole de redirection et si ce n'est pas un pipe 
        si champs valide alors effectue la redirection
        sinon renvoyer le code d'erreur 2 (syntax error)
```

Après la commande exécutée, les redirections sont remises à défaut.

### Gestion des pipelines
Deux commandes reliés par des pipelines ont leur comportement légèrement modifiés : la première commande envoie son résultat à la deuxième commande qui s'en sert comme entrée. Il est possible de rediriger l'entrée standard pour la première commande, et de regiriger la sortie standard pour la dernière commande.

La gestion des pipelines est gérée par plusieurs méthodes. La première méthode, `exec_pipeline` permet de regarder la présence ou non des bonnes redirections aux bons instants. Si une redirection est présente et valide, alors il l'effectue avant d'exécuter la commande. Sinon, il exécute uniquement la commande en remplissant les pipelines de la bonne valeur.

```
Fonction exec_pipeline (tab) :
    A ECRIRE
```

### Gestion des signaux
Le programme `slash` ignore les signaux `SIGINT` et `SIGTERM`, contrairement à ses fils. Pour ce faire, une fonction `treat_signal(bool)` a été écrite, qui ignore ou non les deux signaux selon le boolean donné.

```
Fonction treat_signal (bool) :
    si le boolean est à vrai alors
        ignore les deux signaux
    sinon
        restaure le comportement par défaut des deux signaux
```

Il suffit ainsi d'appeler cette méthode à l'initialisation du programme `slash` pour le faire ignorer ces signaux, et à l'initialisation de ses fils pour qu'ils ne les ignorent pas.