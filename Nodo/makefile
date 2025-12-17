# ==========================================
# MAKEFILE BLINDADO (Tarea 3)
# ==========================================

# Compilador y Flags
CXX = g++
CXXFLAGS = -Wall -pthread
LIBS = -lwiringPi
TARGET = nodo

# Archivos fuente
SRCS = main.cpp capaRed.cpp menu.cpp
OBJS = $(SRCS:.cpp=.o)

# --- REGLA PRINCIPAL ---
all: fix_time $(TARGET)
	@echo "? Compilación exitosa."
	@echo "?? Asignando permisos de ejecución..."
	chmod +x $(TARGET)

# --- REGLA SEGURA DE TIEMPOS ---
# SOLUCIÓN DEL ERROR: Solo tocamos código fuente, NUNCA el log.txt
fix_time:
	@echo "?? Sincronizando tiempos de archivos fuente..."
	@touch *.cpp *.h

# --- LINKEO ---
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# --- COMPILACIÓN INDIVIDUAL ---
main.o: main.cpp capaRed.h menu.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c main.cpp

capaRed.o: capaRed.cpp capaRed.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c capaRed.cpp

menu.o: menu.cpp menu.h capaRed.h structProtocolo.h
	$(CXX) $(CXXFLAGS) -c menu.cpp

# --- EJECUCIÓN LIMPIA ---
run: all
	@echo "?? Limpiando logs antiguos (sudo)..."
	-sudo rm -f log.txt
	@echo "?? Ejecutando NODO..."
	sudo ./$(TARGET)

# --- LIMPIEZA TOTAL ---
clean:
	@echo "??? Borrando archivos compilados y logs..."
	sudo rm -f *.o $(TARGET) log.txt