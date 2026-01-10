.PHONY: all compile

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

compile:
	pyinstaller --onefile veureka.py
	