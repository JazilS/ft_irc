# 🖧 ft_irc - Internet Relay Chat Server

**ft_irc** est une implémentation d'un serveur IRC conforme à la norme C++ 98. Ce projet recrée les bases d'un serveur IRC fonctionnel avec des fonctionnalités essentielles et des extensions supplémentaires pour une expérience utilisateur enrichie.

---

## ✨ Introduction

Internet Relay Chat (IRC) est un protocole de communication textuel utilisé pour des discussions en groupe (canaux) ou des conversations privées. Les clients IRC se connectent à des serveurs IRC pour interagir avec des réseaux. 

Ce projet consiste à développer un serveur IRC entièrement fonctionnel en **C++ 98**, sans utiliser de bibliothèque externe ni de forking, tout en respectant les contraintes de la norme.

---

## 🚀 Fonctionnalités principales

### 🔑 Authentification et gestion des connexions
- **Authentification** via un mot de passe fourni à la connexion.
- Prise en charge de plusieurs clients simultanés.
- Communication basée sur **TCP/IP (v4 ou v6)**.
- Mode non-bloquant grâce à l'utilisation de `poll()` (ou équivalent).

### 📜 Commandes IRC essentielles
- **Authentification utilisateur** : définir un `nickname` et un `username`.
- **Gestion des canaux** :
  - Rejoindre et quitter des canaux.
  - Envoyer et recevoir des messages dans les canaux.
  - Transmettre les messages d'un client à tous les autres clients du même canal.
- **Messagerie privée** : envoyer des messages directs entre utilisateurs.

### 🛡️ Gestion des rôles
- **Opérateurs de canal** :
  - Gestion des utilisateurs du canal avec les commandes suivantes :
    - `KICK` : éjecter un utilisateur.
    - `INVITE` : inviter un utilisateur au canal.
    - `TOPIC` : modifier ou afficher le sujet du canal.
    - `MODE` : changer le mode du canal :
      - `i` : invitation obligatoire pour rejoindre.
      - `t` : restrictions sur la modification du sujet.
      - `k` : définir un mot de passe pour le canal.
      - `o` : attribuer ou retirer le rôle d'opérateur.
      - `l` : limiter le nombre d'utilisateurs.

---

## 🛠️ Technologies utilisées

- **Langage** : C++ 98 - Respect strict de la norme.
- **Communication réseau** : TCP/IP via les fonctions bas-niveau (`socket`, `bind`, `listen`, `accept`, etc.).
- **Gestion des descripteurs** : `poll()` pour gérer les connexions et communications non bloquantes.
- **Système de build** : Makefile avec les règles standard :
  - `all`, `clean`, `fclean`, `re`, et compilation vers un binaire nommé `ircserv`.

---

## 📂 Fichiers de rendu

- `Makefile`
- Fichiers sources : `*.cpp`, `*.h`, `*.hpp`
- Fichier de configuration optionnel

---

## ⚙️ Usage

1. Clonez le dépôt :
   ```bash
   git clone https://github.com/votre-depot/ft_irc.git
   cd ft_irc
