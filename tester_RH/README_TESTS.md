# RequestHandler Test Suite

Suite de tests complète pour le module RequestHandler de votre projet webserv.

## 🚀 Installation Ultra-Rapide

```bash
# Dans ton projet webserv, crée un dossier tester
mkdir tester

# Mets tous les fichiers que je t'ai donnés dans tester/
# (RequestHandlerTester.cpp, Makefile, integration_test.sh, README_TESTS.md)

# C'est tout! Le Makefile va automatiquement chercher tous tes .cpp et .hpp
```

## ⚡ Utilisation (3 commandes max)

```bash
cd tester
make run       # Compile ET lance les tests
```

C'est tout ! Le Makefile fait TOUT automatiquement :
- ✅ Trouve tous tes .cpp dans le dossier parent
- ✅ Trouve tous tes .hpp 
- ✅ Compile tout
- ✅ Lance les tests

### Autres commandes utiles

```bash
make           # Juste compiler
make clean     # Nettoyer les .o
make fclean    # Nettoyer tout
make re        # Recompiler from scratch
```

## 📁 Structure Attendue

```
ton_projet_webserv/
├── RequestHandler.cpp
├── RequestHandler.hpp
├── ServerConfig.cpp
├── ServerConfig.hpp
├── ... (tous tes autres fichiers)
└── tester/                         ← Crée ce dossier
    ├── RequestHandlerTester.cpp    ← Mes fichiers
    ├── Makefile
    ├── integration_test.sh
    └── README_TESTS.md
```

Le testeur va chercher automatiquement tous les fichiers dans `../ ` (le dossier parent).

## 📋 Que Teste le Testeur ?

### Location Matching
- ✅ Correspondance exacte de location
- ✅ Longest prefix matching (choix de la location la plus spécifique)
- ✅ Fallback vers location par défaut
- ✅ Hiérarchies complexes de locations
- ✅ Locations avec plusieurs niveaux de profondeur

### File Path Generation
- ✅ Génération de chemins pour fichiers spécifiques
- ✅ Résolution de l'index pour les répertoires
- ✅ Combinaisons root/index personnalisées vs par défaut
- ✅ Chemins imbriqués
- ✅ Trailing slashes

### CGI Extension Detection
- ✅ Détection d'extensions .php, .py, .cgi
- ✅ Fichiers sans extension
- ✅ Query strings dans l'URI
- ✅ Extensions multiples (tar.gz)

### Upload Type Detection
- ✅ multipart/form-data
- ✅ application/octet-stream
- ✅ Rejet des types non-upload (JSON, text/plain, etc.)

### Edge Cases
- ✅ URIs vides
- ✅ Slashes multiples
- ✅ Chemins très profonds
- ✅ Caractères spéciaux
- ✅ URL encoding

## 📊 Format de Sortie

```
╔════════════════════════════════════════╗
║  RequestHandler Complete Test Suite   ║
╚════════════════════════════════════════╝

=== Testing Location Matching ===
[PASS] Location matching: /api/users (longest match)
[PASS] Location matching: /api (shorter match)
[PASS] Location matching: default location
...

╔════════════════════════════════════════╗
║           Test Results Summary         ║
╚════════════════════════════════════════╝
Tests Passed: 45
Tests Failed: 2
Total Tests: 47

✓ All tests passed!
```

## 🐛 Problèmes de Compilation ?

### Le testeur ne trouve pas tes fichiers ?
Le Makefile cherche dans `../` - assure-toi que `tester/` est bien un sous-dossier de ton projet.

### Erreurs de compilation avec tes classes ?
Le testeur crée des objets HttpRequest avec des membres privés. Deux solutions :

**Solution 1 (rapide)** : Ajoute dans HttpRequest.hpp
```cpp
friend class RequestHandlerTester;
```

**Solution 2** : Le testeur a déjà un accès ami - vérifie juste que toutes tes classes sont bien implémentées.

### Le Makefile compile aussi main.cpp ?
Pas de souci ! Il le filtre automatiquement :
```makefile
FILTERED_SRCS = $(filter-out ../main.cpp, $(PARENT_SRCS))
```

Si tu as d'autres fichiers à exclure, modifie cette ligne dans le Makefile.

## 🧪 Tests d'Intégration (Bonus)

```bash
# Crée un vrai environnement de test avec fichiers
./integration_test.sh

# Nettoie après
./integration_test.sh cleanup
```

## 🎯 Ce Que le Testeur Vérifie

**47 tests** couvrant :
- Matching de locations (simples, complexes, hiérarchiques)
- Génération de paths avec root/index
- Détection d'extensions CGI
- Types d'upload
- Caractères spéciaux dans les URIs
- Edge cases et cas limites
- Copy/assignment operators

## 💡 Tips

- `make run` pour tout faire d'un coup
- Le Makefile est silencieux sauf pour les infos importantes
- Tous les .cpp et .hpp sont trouvés automatiquement
- Si un test échoue, regarde la ligne `[FAIL]` pour savoir lequel

## ⚠️ Note Importante

Le testeur accède à des membres privés de HttpRequest pour créer des requêtes de test. Si ça ne compile pas, ajoute simplement :

```cpp
// Dans HttpRequest.hpp
friend class RequestHandlerTester;
```

Bon courage pour ton webserv! 🚀
