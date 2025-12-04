# Variables del compilador y flags
CXX = g++
# -std=c++0x: Habilita soporte C++11 (necesario para std::stof/stol en GCC antiguo)
# -Wall: Muestra todas las advertencias
CXXFLAGS = -std=c++0x -Wall
# Librerías: WiringPi para control GPIO
LIBS = -lwiringPi

# Nombre del ejecutable final
TARGET = emisor_sincrono

# Archivos fuente y objetos correspondientes
SRCS = main.cpp funcionesMenu.cpp funcionesProtocolo.cpp
OBJS = $(SRCS:.cpp=.o)

# Regla por defecto (se ejecuta al escribir solo 'make')
all: $(TARGET)

# Enlazado: Crea el ejecutable a partir de los objetos
$(TARGET): $(OBJS)
	@echo "Enlazando archivos objeto para crear $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compilación: Crea archivos objeto (.o) desde los fuentes (.cpp)
# Solo recompila si el .cpp ha cambiado
%.o: %.cpp
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla especial: Compila (si es necesario) y ejecuta con sudo
run_program: $(TARGET)
	@echo "----------------------------------------"
	@echo "Ejecutando $(TARGET) con permisos de superusuario..."
	@echo "----------------------------------------"
	sudo ./$(TARGET)

# Limpieza: Borra los archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Utilidad: Actualiza las fechas de los archivos para evitar errores de 'Clock skew'
fix-clock:
	touch *