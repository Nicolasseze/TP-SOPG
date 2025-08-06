# Servidor TCP Clave-Valor (TP SOPG)

Este proyecto es un trabajo práctico para la materia **Sistemas Operativos y Programación de Redes**, que consiste en implementar un **servidor TCP en C** que actúe como una base de datos simple de tipo **clave-valor**, utilizando archivos como almacenamiento persistente.

---

## 🧠 Descripción

El servidor:

- Escucha conexiones TCP en el puerto **5000**
- Acepta comandos ASCII tipo `SET`, `GET` y `DEL` por parte de un cliente
- Crea, lee o borra archivos en el directorio de ejecución
- Finaliza cada conexión después de procesar un solo comando

---

## 📡 Protocolo de comandos

Los comandos válidos que puede enviar el cliente son:
SET <clave> <valor>\n
GET <clave>\n
DEL <clave>\n

### Ejemplos

```bash
# Crear un valor
echo "SET fruta banana" | nc localhost 5000

# Obtener un valor
echo "GET fruta" | nc localhost 5000

# Borrar un valor
echo "DEL fruta" | nc localhost 5000
