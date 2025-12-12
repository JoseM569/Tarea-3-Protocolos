# --- CONFIGURACIÓN COMPATIBLE (Sin C++11) ---
CXX = g++
# Quitamos -std=c++11 para evitar el error
CXXFLAGS = -Wall -pthread
LIBS = -lwiringPi
TARGET = nodo

# --- ARCHIVOS ---
OBJS = main.o capaRed.o menu.o

# --- REGLA PRINCIPAL ---
all: fix_time $(TARGET)
	@echo "✅ Compilación exitosa."
	@echo "🔑 Asignando permisos..."
	chmod +x $(TARGET)

# --- AUTO-CORRECCIÓN DE HORA ---
fix_time:
	@echo "🕒 Ajustando tiempos (Touch)..."
	@touch *

# --- LINKEO ---
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# --- OBJETOS ---
main.o: main.cpp capaRed.h menu.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c main.cpp

capaRed.o: capaRed.cpp capaRed.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c capaRed.cpp

menu.o: menu.cpp menu.h capaRed.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c menu.cpp

# --- RUN ---
run: all
	@echo "🚀 Ejecutando..."
	sudo ./$(TARGET)

clean:
	rm -f *.o $(TARGET)