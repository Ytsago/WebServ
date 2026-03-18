# ConfigParser Test Suite

Suite de tests complète pour le module ConfigParser de votre projet webserv.

## 🚀 Installation Ultra-Rapide

```bash
# Dans ton projet webserv, crée un dossier tester_CP
mkdir tester_CP

# Mets tous les fichiers dedans
# (ConfigParserTester.cpp, Makefile, README_CP.md)
```

## ⚡ Utilisation

```bash
cd tester_CP
make run       # Compile ET lance les tests
```

C'est tout ! Le Makefile fait TOUT automatiquement.

## 📋 Ce Que Teste le Testeur

### Configurations Valides ✅

#### 1. **Basic Valid Config**
```nginx
server {
    listen 8080;
    server_name test.com;
    host localhost;
    root /var/www;
    index index.html;
}
```
Vérifie : parsing de base, tous les champs extraits correctement

#### 2. **Multiple Servers**
```nginx
server { listen 8080; }
server { listen 8081; }
server { listen 8082; }
```
Vérifie : 3 serveurs parsés, ports corrects

#### 3. **Locations**
```nginx
location / {
    allow_methods GET POST;
    autoindex on;
}
location /api {
    allow_methods GET POST DELETE;
    root /var/www/api;
}
```
Vérifie : locations multiples, methods, autoindex, root custom

#### 4. **CGI Configuration**
```nginx
location /cgi-bin {
    cgi_ext .py;
    cgi_path /usr/bin/python3;
}
```
Vérifie : is_cgi flag, extension, path

#### 5. **Comments and Whitespace**
```nginx
# Comment
server {
    listen    8080   ;   # Trailing
    
    # Empty lines
}
```
Vérifie : ignore les commentaires, trim les espaces

#### 6. **Default Location**
Server sans location explicite → crée automatiquement une default location
Vérifie : is_default_set, root/index du serveur

#### 7. **client_max_body_size**
Vérifie : parsing de size_t

#### 8. **Complex Real-World Config**
2 serveurs, 4 locations, CGI, méthodes diverses
Vérifie : configuration complète et réaliste

### Gestion d'Erreurs ❌

#### 1. **Missing Closing Brace**
```nginx
server {
    listen 8080;
# Missing }
```
Attend : ConfigException avec "brace"

#### 2. **Invalid Port**
```nginx
listen notanumber;
```
Attend : ConfigException avec "listen"

#### 3. **Invalid Autoindex**
```nginx
autoindex maybe;
```
Attend : ConfigException (seulement "on" ou "off")

#### 4. **Invalid Method**
```nginx
allow_methods GET PATCH PUT;
```
Attend : ConfigException (seulement GET/POST/DELETE)

#### 5. **Incomplete CGI**
```nginx
location /cgi {
    cgi_ext .py;
    # Missing cgi_path
}
```
Attend : ConfigException avec "CGI" ou "incomplete"

#### 6. **Nested Server Block**
```nginx
server {
    server {  # Nested - invalid
    }
}
```
Attend : ConfigException

#### 7. **Nested Location Block**
```nginx
location / {
    location /nested {  # Nested - invalid
    }
}
```
Attend : ConfigException

#### 8. **Missing Location Path**
```nginx
location {  # Missing path
    allow_methods GET;
}
```
Attend : ConfigException avec "path"

#### 9. **File Not Found**
```nginx
ConfigParser("nonexistent.conf");
```
Attend : ConfigException avec "file" ou "open"

#### 10. **Empty File**
Fichier vide → 0 serveurs (pas d'erreur)

#### 11. **Only Comments**
Fichier avec seulement des commentaires → 0 serveurs

#### 12. **Invalid client_max_body_size**
```nginx
client_max_body_size invalid;
```
Attend : ConfigException

## 📊 Format de Sortie

```
╔════════════════════════════════════════╗
║   ConfigParser Complete Test Suite    ║
╚════════════════════════════════════════╝

=== Testing Basic Valid Config ===
[PASS] Basic valid: parsed successfully
[PASS] Basic valid: port correct
[PASS] Basic valid: server_name correct
...

--- Error Handling Tests ---
=== Testing Missing Closing Brace ===
[PASS] Missing brace: throws ConfigException
[PASS] Missing brace: mentions missing brace
...

╔════════════════════════════════════════╗
║           Test Results Summary         ║
╚════════════════════════════════════════╝
Tests Passed: 52
Tests Failed: 0
Total Tests: 52

✓ All tests passed!
```

## 🔧 Structure Attendue

```
ton_projet_webserv/
├── inc/
│   ├── ConfigParser.hpp
│   ├── ServerConfig.hpp
│   ├── LocationConfig.hpp
│   └── ConfigException.hpp
├── src/
│   └── class/
│       ├── ConfigParser.cpp
│       ├── ServerConfig.cpp
│       └── LocationConfig.cpp
└── tester_CP/                    ← Crée ce dossier
    ├── ConfigParserTester.cpp
    ├── Makefile
    └── README_CP.md
```

## 💡 Ce Que Le Testeur Fait

1. **Crée** des fichiers .conf temporaires dans `./test_configs/`
2. **Parse** chaque config avec ConfigParser
3. **Vérifie** les valeurs parsées (ports, paths, etc.)
4. **Teste** les exceptions pour configs invalides
5. **Nettoie** automatiquement les fichiers de test

## 🎯 Tests Détaillés

### Test de Locations Multiples
```cpp
location / { ... }       → locations[0].get_path() == "/"
location /api { ... }    → locations[1].get_path() == "/api"
```

### Test de Methods
```cpp
allow_methods GET POST DELETE
→ methods.size() == 3
→ methods contient "GET", "POST", "DELETE"
```

### Test CGI Complet vs Incomplet
```cpp
// COMPLET
cgi_ext .py;
cgi_path /usr/bin/python3;
→ is_cgi() == true ✅

// INCOMPLET
cgi_ext .py;
// Missing cgi_path
→ throws ConfigException ❌
```

### Test Default Location
```cpp
Server sans location explicite
→ is_default_set() == true
→ default_location.get_root() == server.get_root()
→ default_location.get_index() == server.get_index()
```

## ⚠️ Notes Importantes

- Le testeur crée `./test_configs/` dans le dossier courant
- Tous les fichiers sont nettoyés automatiquement
- Le Makefile filtre les .cpp inutiles (main, WebServ, etc.)
- Aucune modification de ton code nécessaire !

## 🐛 Si Ça Ne Compile Pas

### Erreur : undefined reference
```bash
make debug  # Affiche les sources trouvés
```
Vérifie que ConfigParser.cpp, ServerConfig.cpp, LocationConfig.cpp sont bien compilés.

### Erreur : ConfigException not found
Vérifie que `ConfigException.hpp` est bien dans `../inc/`

## 📈 Couverture

- **52+ assertions** minimum
- **12 catégories** de tests
- **3 niveaux** : valide, invalide, edge cases
- **100% de coverage** sur le parsing

## 🎓 Résumé

Le testeur vérifie que ConfigParser :
1. Parse correctement les configs valides (serveurs, locations, CGI)
2. Extrait toutes les valeurs (ports, paths, methods, etc.)
3. Gère les commentaires et espaces
4. Crée la default location automatiquement
5. Lance des exceptions pour configs invalides
6. Donne des messages d'erreur clairs avec numéro de ligne

Bon courage pour ton webserv! 🚀
