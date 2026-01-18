# 🚀 Veureka

**Un linguaggio di programmazione moderno, semplice ma potente**

<img width="200" height="200" alt="logo" src="https://github.com/user-attachments/assets/cef0f1c4-dc8d-416c-aaa4-2b4c9289173e" />

Veureka è un linguaggio interpretato con sintassi pulita che supporta programmazione funzionale, orientata agli oggetti, closures e lambda functions.


[![Python](https://img.shields.io/badge/Python-3.7+-blue.svg)](https://www.python.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

## ✨ Caratteristiche

- 🎯 **Sintassi pulita e intuitiva**
- 🔧 **Programmazione funzionale** (lambda, map, filter, reduce)
- 🏗️ **Programmazione orientata agli oggetti** (classi, metodi, self)
- 🔄 **Closures** e funzioni di ordine superiore
- ⚡ **Operatori compatti** (+=, -=, ++, --)
- 📦 **Collezioni native** (liste, mappe/dizionari)
- 💬 **Input/Output interattivo** (input da terminale, print con più argomenti)
- 📚 **Sistema di librerie** (include per importare file .ver)
- 🔨 **Compilatore integrato** - Genera binari ELF standalone
- 🐍 **Implementato in Python puro** (nessuna dipendenza esterna)

## 📥 Installazione

### Opzione 1: Usa Python direttamente
```bash
# Clone del repository
git clone https://github.com/vincenzofranchino/veureka-lang.git
cd veureka-lang

# Esegui direttamente
python veurekabeta.py script.ver

# Compila a binario
python veurekabeta.py -c script.ver -o app
```

### Opzione 2: Installa come comando di sistema (Linux)
```bash
git clone https://github.com/vincenzofranchino/veureka-lang.git
cd veureka-lang

make build
make install

# Adesso puoi usare da qualsiasi directory:
veureka script.ver
veureka -c script.ver -o app

# Per disinstallare:
make clean
```

### Dipendenze
- Python 3.7+
- Per la compilazione a binari: `gcc` (di solito preinstallato su Linux)

## 🚀 Quick Start

### Hello World
```veureka
print("Ciao, mondo!")
```

### Eseguire un file
```bash
python veurekabeta.py hello.ver
# oppure (se installato)
veureka hello.ver
```

### Compilare a Binario ELF
```bash
# Compila a eseguibile
python veurekabeta.py -c script.ver -o app

# Esegui il binario
./app
```

### REPL Interattivo
```bash
python veurekabeta.py
# oppure (se installato)
veureka
```

### Esempi dimostrativi
```bash
python veurekabeta.py --examples
# oppure
veureka --examples
```

## 📖 Sintassi di Base

### Variabili
```veureka
let nome = "Mario"
let età = 25
let pi = 3.14159
```

### Funzioni
```veureka
fn saluta(nome)
    print("Ciao, " + nome + "!")
end

saluta("Alice")
```

### Lambda Functions
```veureka
let doppio = fn(x) => x * 2
let somma = fn(a, b) => a + b

print(doppio(5))     # 10
print(somma(3, 7))   # 10
```

### Strutture di Controllo
```veureka
# If-Elif-Else
if x > 10
    print("Grande")
elif x > 5
    print("Medio")
else
    print("Piccolo")
end

# While
let i = 0
while i < 5
    print(i)
    i += 1
end

# For
for n in range(1, 11)
    print(n)
end
```

### Classi e OOP
```veureka
class Persona
    fn __init__(nome, età)
        self.nome = nome
        self.età = età
    end
    
    fn saluta()
        print("Ciao, sono " + self.nome)
    end
    
    fn compleanno()
        self.età += 1
    end
end

let mario = new Persona("Mario", 25)
mario.saluta()        # "Ciao, sono Mario"
mario.compleanno()
```

### Operatori Compatti
```veureka
let x = 10

x += 5    # x = 15
x -= 3    # x = 12
x *= 2    # x = 24
x /= 4    # x = 6

# Incremento/Decremento
x++       # postfisso
++x       # prefisso
x--       # postfisso
--x       # prefisso
```

### Collezioni

#### Liste
```veureka
let numeri = [1, 2, 3, 4, 5]

for n in numeri
    print(n * 2)
end

print(len(numeri))    # 5
print(numeri[0])      # 1
```

#### Mappe/Dizionari
```veureka
let utente = {
    nome: "Alice",
    età: 30,
    email: "alice@example.com"
}

print(utente["nome"])    # "Alice"
```

### Funzioni di Ordine Superiore
```veureka
let numeri = [1, 2, 3, 4, 5]

# Map
let doppi = map(numeri, fn(n) => n * 2)
print(doppi)  # [2, 4, 6, 8, 10]

# Filter
let pari = filter(numeri, fn(n) => n % 2 == 0)
print(pari)   # [2, 4]

# Reduce
let somma = reduce(numeri, fn(acc, n) => acc + n, 0)
print(somma)  # 15
```

### Closures
```veureka
fn crea_contatore()
    let count = 0
    return fn() => count = count + 1
end

let contatore = crea_contatore()
print(contatore())  # 1
print(contatore())  # 2
print(contatore())  # 3
```

### Input da Terminale
```veureka
print("Inserisci il tuo nome:")
let nome = input()
print("Ciao, " + nome + "!")

# Lettura di numeri
print("Inserisci un numero:")
let numero = int(input())
print("Il doppio è:", numero * 2)
```

### Librerie e Include
```veureka
# File: lib/math_utils.ver
fn quadrato(x)
    return x * x
end

fn ipotenusa(a, b)
    return (a * a + b * b) ** 0.5
end
```

```veureka
# File: main.ver
include "lib/math_utils"

print("Quadrato di 5:", quadrato(5))
print("Ipotenusa(3,4):", ipotenusa(3, 4))
```

## 🎯 Esempi Completi

### FizzBuzz
```veureka
for i in range(1, 101)
    if i % 15 == 0
        print("FizzBuzz")
    elif i % 3 == 0
        print("Fizz")
    elif i % 5 == 0
        print("Buzz")
    else
        print(i)
    end
end
```

### Fibonacci Ricorsivo
```veureka
fn fibonacci(n)
    if n < 2
        return n
    end
    return fibonacci(n - 1) + fibonacci(n - 2)
end

print(fibonacci(10))  # 55
```

### Classe Contatore
```veureka
class Contatore
    fn __init__(valore)
        self.valore = valore
    end
    
    fn incrementa()
        self.valore++
        return self.valore
    end
    
    fn reset()
        self.valore = 0
    end
end

let c = new Contatore(0)
print(c.incrementa())  # 1
print(c.incrementa())  # 2
c.reset()
```

### Calcolatrice con Method Chaining
```veureka
class Calc
    fn __init__()
        self.risultato = 0
    end
    
    fn add(n)
        self.risultato += n
        return self
    end
    
    fn mul(n)
        self.risultato *= n
        return self
    end
    
    fn get()
        return self.risultato
    end
end

let calc = new Calc()
let result = calc.add(10).mul(5).add(3).get()
print(result)  # 53
```

## 📚 Funzioni Built-in

| Funzione | Descrizione |
|----------|-------------|
| `print(...)` | Stampa valori |
| `len(collection)` | Lunghezza di una collezione |
| `range(start, stop, step?)` | Genera sequenza di numeri |
| `str(value)` | Converte in stringa |
| `int(value)` | Converte in intero |
| `float(value)` | Converte in decimale |
| `type(value)` | Ritorna il tipo |
| `map(collection, fn)` | Applica funzione a ogni elemento |
| `filter(collection, fn)` | Filtra elementi |
| `reduce(collection, fn, init?)` | Riduce a singolo valore |
| Funzione | Descrizione |
|----------|-------------|
| `print(...)` | Stampa valori |
| `input()` | Legge input da terminale (ritorna stringa) |
| `input(prompt)` | Legge input con messaggio |
| `len(collection)` | Lunghezza di una collezione |
| `range(start, stop, step?)` | Genera sequenza di numeri |
| `str(value)` | Converte in stringa |
| `int(value)` | Converte in intero |
| `float(value)` | Converte in decimale |
| `type(value)` | Ritorna il tipo |
| `map(collection, fn)` | Applica funzione a ogni elemento |
| `filter(collection, fn)` | Filtra elementi |
| `reduce(collection, fn, init?)` | Riduce a singolo valore |
| `sum(collection)` | Somma elementi |
| `max(collection)` | Valore massimo |
| `min(collection)` | Valore minimo |
| `abs(number)` | Valore assoluto |

### Statement Speciali
```veureka
# Include - Importa una libreria
include "lib/math_utils"
include("string_utils")    # Entrambe le sintassi sono valide

# Break - Esce da un ciclo
for i in range(1, 100)
    if i == 10
        break
    end
    print(i)
end

# Continue - Salta all'iterazione successiva
for i in range(1, 11)
    if i % 2 != 0
        continue
    end
    print(i)  # Stampa solo numeri pari
end
```

## 🛠️ Uso del REPL

```bash
$ python veurekabeta.py
============================================================
Veureka REPL - Linguaggio di Programmazione Interattivo
============================================================
Digita 'exit' o 'quit' per uscire
Digita 'help' per vedere i comandi disponibili

ver> let x = 10
ver> x += 5
15
ver> print(x)
15
ver> fn double(n) => n * 2
ver> double(x)
30
ver> vars
Variabili globali:
  x = 15
  double = <function>
ver> exit
Arrivederci!
```

### Comandi REPL
- `exit` / `quit` - Esci dal REPL
- `help` - Mostra aiuto
- `clear` - Pulisci le variabili
- `vars` - Mostra tutte le variabili definite

## 🔨 Compilazione a Binari ELF

Veureka include un compilatore integrato che genera **veri binari ELF** da codice Veureka. Il binario risultante è standalone e non richiede Python installato sulla macchina di destinazione.

### Compilare un Programma

```bash
# Compila script.ver a binario eseguibile
python veurekabeta.py -c script.ver -o app

# Esegui il binario
./app
```

### Verificare il Binario

```bash
# Mostra il tipo di file
file ./app
# Output: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, ...

# Verificare le dimensioni
ls -lh ./app
```

### Esempio Completo

```bash
# 1. Crea il programma
cat > hello.ver << 'EOF'
print("Ciao dal binario compilato!")
let numeri = [1, 2, 3, 4, 5]
for n in numeri
    print(n * 2)
end
EOF

# 2. Compila
python veurekabeta.py -c hello.ver -o hello_app

# 3. Esegui
./hello_app
```

### Vantaggi della Compilazione

- ✅ **Binario standalone** - Non richiede Python o dipendenze esterne
- ✅ **Esecuzione veloce** - Codice compilato a machine code tramite Nuitka
- ✅ **Distribuzione facile** - Un singolo file executable
- ✅ **Protezione del codice sorgente** - Il sorgente Veureka è compilato nel binario

### TodoList con Classe
```veureka
class TodoList
    fn __init__()
        self.tasks = []
    end
    
    fn add(task)
        self.tasks = self.tasks + [task]
        print("✓ Aggiunto: " + task)
    end
    
    fn show()
        print("\n=== TODO LIST ===")
        for i in range(len(self.tasks))
            print(str(i + 1) + ". " + self.tasks[i])
        end
    end
    
    fn count()
        return len(self.tasks)
    end
end

let todo = new TodoList()
todo.add("Studiare Veureka")
todo.add("Scrivere codice")
todo.show()
```

### Punto 2D con Metodi
```veureka
class Punto
    fn __init__(x, y)
        self.x = x
        self.y = y
    end
    
    fn distanza()
        return (self.x ** 2 + self.y ** 2) ** 0.5
    end
    
    fn muovi(dx, dy)
        self.x += dx
        self.y += dy
    end
end

let p = new Punto(3, 4)
print(p.distanza())  # 5.0
p.muovi(1, 1)
print(p.distanza())  # 7.07...
```

## 🎯 Roadmap

- [x] ✅ Input da terminale
- [x] ✅ Sistema di librerie (include)
- [x] ✅ **Compilazione a binari ELF standalone**
- [ ] Gestione eccezioni (try/catch/finally)
- [ ] Ereditarietà tra classi
- [ ] Decoratori
- [ ] Generatori e iteratori custom
- [ ] Comprensione liste `[x * 2 for x in range(10)]`
- [ ] Pattern matching avanzato (match/case migliorato)
- [ ] Type hints opzionali
- [ ] Package manager
- [ ] Moduli con namespace

## 🤝 Contribuire

I contributi sono benvenuti! Ecco come puoi aiutare:

1. Fork del progetto
2. Crea un branch per la tua feature (`git checkout -b feature/AmazingFeature`)
3. Commit dei cambiamenti (`git commit -m 'Add some AmazingFeature'`)
4. Push al branch (`git push origin feature/AmazingFeature`)
5. Apri una Pull Request

## 📄 Licenza

Questo progetto è distribuito sotto licenza MIT. Vedi il file `LICENSE` per maggiori dettagli.

## 👨‍💻 Autore

**Vincenzo Franchino**

- GitHub: [@vincenzofranchino](https://github.com/vincenzofranchino)


