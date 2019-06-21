# Projet Système – Shell

Antoine Julien et Quentin Lapierre

## Compilation

Un makefile est fourni, compiler tout simplement en utilisant la commande `make`.  

## Utilisation et options

Possibilité de taper des arguments sur plusieurs lignes et de taper une suite de commande sur plusieurs lignes comme dans le bash. 

### Commande internes 

La définition de toutes les commandes internes sauf `cd`et `exit`sont présentes dans les fichiers commandesInterne.  

Liste de commandes utilisables dans notre shell: 

- `cat`affiche sur la sortie standard le contenu du fichier passé en paramètre; utilisation:

  ```bash
  cat <path> 
  ```

* `cp` sert à copier des fichiers. On peut aussi bien copier un fichier donné vers une destination précise que copier un ensemble de fichiers dans un répertoire. Si le premier argument correspond à un répertoire `cp` va copier l'ensemble du contenu du répertoire de façon récursive. Sinon si le premier argument est un fichier il va le copier vers sa destination; utilisation:

  ```bash
  cp <source> <destination>
  ```

*  `ls` affiche l'ensemble des fichiers et dossiers contenus dans le répertoire. Il prend en charge les options suivantes:

  * `-a` Afficher tous les fichiers des répertoires, y compris les fichiers commençant par un '.'.
  * `-l`  En  plus  du  nom, affiche le type du fichier, les permissions d'accès, le nombre de liens  physiques, le  nom  du propriétaire et du groupe, la taille en octets.

  ```````bash
  ls <path> -<options>
  ```````

* `find` sans options elle affiche l'ensemble des noms de fichiers qui contiennent le pattern. Elle prend en charge les options suivantes:

  * `-name` trouve la première occurrence qui correspond exactement au pattern.
  * `-exec` implémenté mais buggé

  ```bash
  find <path> <expression>
  ```

* `exit ` quitte le shell

### Redirections 

Possibilité d’utiliser les trois redirections standards : `<`,` >` et `2>` qui permettent de rediriger respectivement l’entrée standard (stdin), la sortie standard (stdout), et la sortie d’erreur (stderr).

### Pipes 

## Bugs

1. `-exec` de find ne marche pas
2. `/bin/echo coucou < test.txt > cat` ne marche pas
3. `cd`ne change pas le cwd car il est exécuté dans un processus fils
4. `setenv` disponible pour changer des variables environnement mais problème.
5. Bien d'autres bugs...

## Améliorations et essais

### Essais

* Travailler avec les variables d'environnement
* Utiliser l'option `-exec` sur find 

### Améliorations

* Réparer les fonctionnalités existantes
* Changer la gestion d'erreur
* Réfléchir à une manière de gérer un plus grand nombres de commandes internes. 
* Repenser la structure notre shell pour une meilleure utilisation



