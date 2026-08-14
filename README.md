# Sord IR — Convolution Reverb Plugin v1.5

VST3 / AU — macOS 12+ (Universal Binary: Apple Silicon + Intel)

---

## Build

### Prerequis
- macOS 12+ avec Xcode 14+
- CMake 3.22+ (`brew install cmake`)
- Git

### IRs

Les fichiers IR (.wav) ne sont pas inclus dans ce depot. Placez vos propres IRs dans `Resources/IRs/` avant de compiler, et mettez a jour `CMakeLists.txt` en consequence.

### Compiler

```bash
cd SordIR
bash build.sh
```

Le script va automatiquement :
- Cloner JUCE 8.0.4
- Compiler en Universal Binary (ARM64 + x86_64)
- Signer en ad-hoc

---

## Parametres

| Parametre   | Defaut | Plage         |
|-------------|--------|---------------|
| Pre-Delay   | 0 ms   | 0-100 ms      |
| Size        | 1.00x  | 0.25x - 4x    |
| Dry/Wet     | 100%   | 0-100%        |
| Gain Out    | 0 dB   | -24 - +12 dB  |
| EQ On       | true   | —             |
| EQ x4 bands | 0 dB   | +-12 dB       |

## Categories IR

**Brass** — Trompette (Straight, Bucket, Cup, Harmon) · Trombone (Straight, Bucket, Cup, Harmon, Harmon Stem) · Cor (Straight, Brass Stop) · Tuba Straight  
**Strings** — Sordino  
**Spaces** — Studio Depth · Concert Hall  
**Bricasti** — Large Hall · Brass Hall · Amsterdam · Boston · Worcester · Mechanics Hall · Saint Gerold

## Import IR custom

- Bouton **IMPORT IR** dans l'interface
- Ou drag & drop d'un fichier .wav/.aif sur la fenetre du plugin
