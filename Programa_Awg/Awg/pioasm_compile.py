import os
from SCons.Script import Import

# Import the environment (provided by PlatformIO)
Import("env")

def pio_compile(source, target, env):
    # Define las rutas del archivo .pio y el archivo .h de salida
    pio_program = "src/Awg.pio"
    pio_header = "include/Awg.pio.h"
    
    print(f"Compilando {pio_program} a {pio_header}")
    
    # Ejecuta el comando pioasm para compilar .pio a .h
    result = os.system(f"pioasm.exe {pio_program} {pio_header}")
    
    if result == 0:
        print(f"Compilación exitosa: {pio_header} generado")
    else:
        print("Error en la compilación del archivo PIO")

# Ejecutar el compilador PIO antes de la compilación general
env.AddPreAction("build", pio_compile)