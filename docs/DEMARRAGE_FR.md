# Prendre en main HuffLab (guide en français)

## 1. Ce qui vient du TP

Les algorithmes que tu as codés sont **conservés** dans `src/huffman.cpp` :

- `count_bytes`, `count_distinct`, `compute_entropy`
- `build_tree`, `measure_codes`, `count_bits`
- `encode_rle`, `decode_rle`
- `make_sierpinski`, `make_xor_texture`

Les tests de ton TP sont dans `tests/test_huffman.cpp` ; l'ancien `main.cpp`
qui affiche le tableau des expériences est dans `examples/lab_report.cpp`.
Le fichier `huffman.hpp` est maintenant dans `include/hufflab/`.

## 2. Ce qui est nouveau

- `src/bitstream.cpp` : écrire/lire les bits individuellement.
- `src/codec.cpp` : produire une archive autonome, et la décoder.
- `src/main.cpp` : commandes `compress`, `decompress`, `analyze`, `benchmark`.
- `tests/test_codec.cpp` : tests de reconstruction (round-trip) et d'erreurs.
- `.github/workflows/ci.yml` : tests automatisés sur Linux et Windows.

### L'encodage Huffman en 6 opérations

1. Compter les occurrences de chaque octet (`count_bytes`).
2. Construire l'arbre (`build_tree`).
3. Parcourir l'arbre pour attribuer un code binaire à chaque symbole.
4. Écrire les métadonnées nécessaires au décodage (fréquences et taille initiale).
5. Écrire chaque code bit par bit, en regroupant 8 bits dans un octet.
6. Compléter le dernier octet avec des zéros si nécessaire.

Lors du décodage, les fréquences dans le fichier permettent de reconstruire
**exactement le même arbre** (grâce au départage déterministe dans ton TP).
On suit ensuite l'arbre selon les bits jusqu'à retrouver chaque symbole.

## 3. Compiler et tester

Depuis le dossier `HuffLab` :

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

Sous Linux ou WSL :

```sh
./build/hufflab compress sample.txt sample.huf
./build/hufflab decompress sample.huf restored.txt
cmp sample.txt restored.txt
./build/hufflab analyze sample.txt
./build/hufflab benchmark sample.txt
./build/hufflab_lab_report
```

Sur Windows avec Visual Studio, remplace `./build/hufflab` par
` .\\build\\Release\\hufflab.exe` (en PowerShell), selon ton générateur CMake.
Pour vérifier les deux fichiers, `Compare-Object` est possible mais la
comparaison des octets est plus fiable (par exemple via un hash SHA-256).

Tu peux choisir la méthode :

```sh
./build/hufflab compress sample.txt sample-rle.huf --method rle
./build/hufflab compress sample.txt sample-combined.huf --method combined
```

**Important :** sur de petits fichiers, l'en-tête de l'archive peut être plus
gros que les données compressées. Un gain négatif est donc normal.

## 4. Ordre de lecture du code

Pour apprendre, commence par :

1. `include/hufflab/codec.hpp` : l'API publique, trois méthodes possibles.
2. `src/bitstream.cpp` : transformer une suite de bits en octets.
3. `src/codec.cpp`, fonction `compress` : comment ton arbre est utilisé.
4. `src/codec.cpp`, fonction `decompress` : reconstruire et parcourir l'arbre.
5. `tests/test_codec.cpp` : pourquoi un **round-trip** est essentiel.
6. `src/main.cpp` : gérer les arguments et les erreurs.

## 5. Prochaines évolutions à réaliser toi-même

- Afficher l'arbre Huffman comme un graphique (Graphviz ou Qt).
- Comparer des fichiers de tailles variées (texte, images, données aléatoires).
- Ajouter un CRC32 pour détecter les archives endommagées.
- Réduire la taille de l'en-tête avec des codes de Huffman canoniques.
- Traiter les gros fichiers en streaming plutôt que de tout charger en RAM.
- Mesurer le temps avec plusieurs exécutions et rapporter la médiane.

## 6. Publier le repository

**Vérifie d'abord les conditions de ton cours** : l'énoncé, les fichiers
fournis et certains tests peuvent appartenir à l'établissement ou à leur
auteur. Si la publication du code fourni n'est pas permise, il faudra
réécrire ces morceaux ou obtenir une autorisation avant de créer un repo public.

Ensuite :

```sh
git init
git add .
git commit -m "Initial release: HuffLab compression engine"
git branch -M main
git remote add origin https://github.com/TON_COMPTE/HuffLab.git
git push -u origin main
```

Le dépôt GitHub vide doit avoir été créé au préalable.
