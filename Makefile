.PHONY: all compile
APP_NAME = veureka
SRC = veureka.py
all:
	@echo "For the compilation of veureka use the command: 'make compile'"
	@echo "=============================================================="
	@echo "Veureka - Linguaggio di Programmazione"
	@echo "========================================="
	@echo "Comandi disponibili:"
	@echo "  make          - Mostra questo messaggio"
	@echo "  make compile  - Compila il progetto con PyInstaller"
	@echo "  make run      - Esegue il compilatore"
	@echo "  make clean    - Rimuove i file di build"
	@echo ""
	@echo "Per eseguire un file .ver: python compilatorecompleto.py tuofile.ver but if the file is compiled use: ./compilatorecompleto tuofile.ver(Linux)"

build:
	pyinstaller --onefile --name $(APP_NAME) $(SRC)

# Installa il binario generato nel sistema
install: build
	install -m 755 dist/$(APP_NAME) /usr/local/bin/

clean:
	rm -rf build dist $(APP_NAME).spec
	
