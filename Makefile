.PHONY: all build clean install installpython uninstall test
APP_NAME = veureka
SRC_C = veureka.c
SRC_PY = veureka.py

all:
	@echo "=============================================================="
	@echo "Veureka - Linguaggio di Programmazione"
	@echo "========================================="
	@echo "Comandi disponibili:"
	@echo "  make build        - Compila la versione C"
	@echo "  make python       - Compila con PyInstaller (versione Python)"
	@echo "  make install      - Installa il binario C nel sistema"
	@echo "  make installpython- Compila e installa il binario Python"
	@echo "  make uninstall    - Rimuove il binario dal sistema"
	@echo "  make test         - Esegue test di base"
	@echo "  make clean        - Rimuove i file di build"
	@echo ""
	@echo "Per eseguire:"
	@echo "  ./veureka script.ver    # Versione C compilata"
	@echo "  python veureka.py       # Versione Python"
	@echo ""
	@echo "Nota: PyInstaller è richiesto per 'make python' e 'make installpython'"

build:
	gcc -o $(APP_NAME) $(SRC_C) -lm
	@echo "✓ Compilazione C completata: ./$(APP_NAME)"

python:
	@if command -v pyinstaller >/dev/null 2>&1; then \
		pyinstaller --onefile --name $(APP_NAME) $(SRC_PY); \
		echo "✓ Compilazione Python completata: dist/$(APP_NAME)"; \
	else \
		echo "PyInstaller non trovato. Installazione in corso..."; \
		pip install pyinstaller; \
		pyinstaller --onefile --name $(APP_NAME) $(SRC_PY); \
		echo "✓ Compilazione Python completata: dist/$(APP_NAME)"; \
	fi

install: build
	install -m 755 $(APP_NAME) /usr/local/bin/
	@echo "✓ Installato in /usr/local/bin/$(APP_NAME)"

uninstall:
	rm -f /usr/local/bin/$(APP_NAME)
	@echo "✓ Disinstallato $(APP_NAME) da /usr/local/bin/"

installpython: python
	install -m 755 dist/$(APP_NAME) /usr/local/bin/
	@echo "✓ Installato Python in /usr/local/bin/$(APP_NAME)"

test: build
	@echo "Testing Veureka C implementation..."
	@echo 'print("Hello from Veureka!")' | ./$(APP_NAME)
	@echo ""
	@echo 'let x = 10\nprint("x =", x)' | ./$(APP_NAME)

clean:
	rm -rf build dist $(APP_NAME).spec $(APP_NAME)
	@echo "✓ File di build rimossi"
	
