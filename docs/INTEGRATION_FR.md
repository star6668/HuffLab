# HuffLab comme bibliothèque C++ : première version

## Ce qui change

L'ancien `HuffLab` proposait déjà une bibliothèque interne (`hufflab_core`)
utilisée par son CLI. Cette nouvelle version expose deux API installables :

1. `hufflab::compress` et `hufflab::decompress` pour des octets en mémoire
   (`include/hufflab/codec.hpp`).
2. `hufflab::compress_file` et `hufflab::decompress_file` pour des chemins
   de fichiers (`include/hufflab/file.hpp`). Ces fonctions retournent les
   tailles d'entrée et de sortie et propagent les erreurs avec des exceptions.

L'outil CLI utilise lui aussi cette API fichier : il ne duplique plus les
opérations de compression/décompression. **Les algorithmes du TP sont conservés.**

## Compiler et tester sous Linux ou WSL

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## Installer la bibliothèque et vérifier l'import dans un autre projet

```sh
cmake --install build --config Release --prefix "$PWD/install"
cmake -S examples/consumer -B consumer-build -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build consumer-build --config Release
ctest --test-dir consumer-build --build-config Release --output-on-failure
```

Le petit exemple `examples/consumer/main.cpp` est **un projet indépendant**.
Il utilise `find_package(HuffLab 0.2 CONFIG REQUIRED)` pour charger le paquet
installé et lie `HuffLab::hufflab`, sans compiler le code source de HuffLab.

Pour l'intégrer à une autre application, regarde son fichier `CMakeLists.txt`.

## Pourquoi c'est une première étape professionnelle

- **API publique réduite** : deux headers documentés au lieu d'exposer tous
  les détails internes de l'arbre de Huffman.
- **Séparation bibliothèque / outil CLI** : chacun peut être intégré ou
  testé séparément.
- **Intégration CMake** : un développeur externe peut réutiliser le paquet
  avec une seule cible `HuffLab::hufflab`.
- **Tests indépendants** : l'exemple consommateur vérifie qu'on n'a pas
  accidentellement dépendu d'un fichier présent uniquement dans le dépôt.

## Ce qui reste à faire plus tard

Le moteur charge toujours les fichiers entièrement en mémoire. Pour les gros
fichiers, il faudra introduire une compression par blocs / streaming, puis
mesurer ses performances. Les archives n'ont pas encore de somme de contrôle.
Ne considère pas cette version comme adaptée aux données critiques.
