#include <iostream>
#include <mysql.h>
#include <string>

using namespace std;

MYSQL* conectar;

struct Banco {
    int id;
    string nombre;
};

Banco bancos[] = {
    {1, "Banco Industrial"},
    {2, "Banrural"},
    {3, "BAC Credomatic"}
};

void mostrarBancos() {
    cout << "\n--- Bancos Disponibles ---\n";
    for (const Banco& b : bancos) {
        cout << b.id << ". " << b.nombre << endl;
    }
}

class Cuenta {
public:
    void crear() {
        string nombre, tipo;
        int tipoOpcion;

        cout << "Ingrese su nombre: ";
        cin.ignore();
        getline(cin, nombre);

        cout << "Seleccione el tipo de cuenta:\n";
        cout << "1. Monetaria\n";
        cout << "2. Ahorro\n";
        cout << "Opcion: ";
        cin >> tipoOpcion;

        if (tipoOpcion == 1) tipo = "Monetaria";
        else if (tipoOpcion == 2) tipo = "Ahorro";
        else {
            cout << "Opcion inválida.\n";
            return;
        }

        string query = "INSERT INTO cuentas(nombre, tipo, saldo) VALUES('" + nombre + "', '" + tipo + "', 0)";
        if (mysql_query(conectar, query.c_str()) == 0)
            cout << "Cuenta creada exitosamente.\n";
        else
            cout << "Error al crear cuenta: " << mysql_error(conectar) << "\n";
    }

    void ver() {
        MYSQL_ROW fila;
        MYSQL_RES* resultado;

        if (mysql_query(conectar, "SELECT * FROM cuentas") == 0) {
            resultado = mysql_store_result(conectar);
            cout << "\n--- Lista de Cuentas ---\n";
            while ((fila = mysql_fetch_row(resultado))) {
                cout << "ID: " << fila[0] << " | Nombre: " << fila[1] << " | Tipo: " << fila[2] << " | Saldo: Q" << fila[3] << "\n";
            }
        }
        else {
            cout << "Error al recuperar cuentas: " << mysql_error(conectar) << "\n";
        }
    }

    void ingresarSaldo() {
        int id;
        double monto;

        ver();
        cout << "\nIngrese el ID de la cuenta a la que desea ingresar saldo: ";
        cin >> id;
        cout << "Ingrese el monto a ingresar: Q";
        cin >> monto;

        string query = "UPDATE cuentas SET saldo = saldo + " + to_string(monto) + " WHERE id = " + to_string(id);
        if (mysql_query(conectar, query.c_str()) == 0) {
            // Registrar movimiento
            string mov = "INSERT INTO movimientos(cuenta_id, tipo, descripcion, monto) VALUES(" + to_string(id) + ", 'Ingreso', 'Ingreso de saldo', " + to_string(monto) + ")";
            mysql_query(conectar, mov.c_str());

            cout << "Saldo ingresado correctamente.\n";
        }
        else {
            cout << "Error al ingresar saldo: " << mysql_error(conectar) << "\n";
        }
    }

    void transferirEntre() {
        int origenId, destinoId;
        double monto;

        ver();
        cout << "\nIngrese el ID de la cuenta de origen: ";
        cin >> origenId;
        cout << "Ingrese el ID de la cuenta destino: ";
        cin >> destinoId;

        if (origenId == destinoId) {
            cout << "No puede transferir a la misma cuenta.\n";
            return;
        }

        cout << "Ingrese el monto a transferir: Q";
        cin >> monto;

        string consulta = "SELECT saldo FROM cuentas WHERE id = " + to_string(origenId);
        mysql_query(conectar, consulta.c_str());
        MYSQL_RES* resultado = mysql_store_result(conectar);
        MYSQL_ROW fila = mysql_fetch_row(resultado);

        if (!fila) {
            cout << "Cuenta origen no encontrada.\n";
            return;
        }

        double saldoOrigen = stod(fila[0]);
        if (saldoOrigen < monto) {
            cout << "Saldo insuficiente.\n";
            return;
        }

        string query1 = "UPDATE cuentas SET saldo = saldo - " + to_string(monto) + " WHERE id = " + to_string(origenId);
        string query2 = "UPDATE cuentas SET saldo = saldo + " + to_string(monto) + " WHERE id = " + to_string(destinoId);

        if (mysql_query(conectar, query1.c_str()) == 0 && mysql_query(conectar, query2.c_str()) == 0) {
            string mov1 = "INSERT INTO movimientos(cuenta_id, tipo, descripcion, monto) VALUES(" + to_string(origenId) + ", 'Transferencia', 'Transferencia a cuenta " + to_string(destinoId) + "', -" + to_string(monto) + ")";
            string mov2 = "INSERT INTO movimientos(cuenta_id, tipo, descripcion, monto) VALUES(" + to_string(destinoId) + ", 'Transferencia', 'Recibido de cuenta " + to_string(origenId) + "', " + to_string(monto) + ")";
            mysql_query(conectar, mov1.c_str());
            mysql_query(conectar, mov2.c_str());

            cout << "Transferencia realizada exitosamente.\n";
        }
        else {
            cout << "Error al realizar transferencia: " << mysql_error(conectar) << "\n";
        }
    }

    void retirarEnCajero() {
        int id;
        double monto;

        ver();
        cout << "\nIngrese el ID de la cuenta de la que desea retirar: ";
        cin >> id;
        cout << "Ingrese el monto a retirar: Q";
        cin >> monto;

        string consulta = "SELECT saldo FROM cuentas WHERE id = " + to_string(id);
        if (mysql_query(conectar, consulta.c_str()) != 0) {
            cout << "Error al buscar cuenta: " << mysql_error(conectar) << "\n";
            return;
        }

        MYSQL_RES* resultado = mysql_store_result(conectar);
        MYSQL_ROW fila = mysql_fetch_row(resultado);

        if (!fila) {
            cout << "Cuenta no encontrada.\n";
            return;
        }

        double saldo = stod(fila[0]);
        if (saldo < monto) {
            cout << "Saldo insuficiente para realizar el retiro.\n";
            return;
        }

        string update = "UPDATE cuentas SET saldo = saldo - " + to_string(monto) + " WHERE id = " + to_string(id);
        if (mysql_query(conectar, update.c_str()) == 0) {
            string mov = "INSERT INTO movimientos(cuenta_id, tipo, descripcion, monto) VALUES(" +
                to_string(id) + ", 'Retiro', 'Retiro en cajero', -" + to_string(monto) + ")";
            mysql_query(conectar, mov.c_str());

            cout << "Retiro realizado exitosamente.\n";
        }
        else {
            cout << "Error al realizar el retiro: " << mysql_error(conectar) << "\n";
        }
    }
};

class CuentaTercero {
public:
    void agregar() {
        string nombre, numeroCuenta;
        int bancoOpcion;

        cin.ignore();
        cout << "Ingrese el nombre del titular: ";
        getline(cin, nombre);

        mostrarBancos();
        cout << "Seleccione el banco: ";
        cin >> bancoOpcion;

        string bancoElegido;
        bool bancoValido = false;
        for (const Banco& b : bancos) {
            if (b.id == bancoOpcion) {
                bancoElegido = b.nombre;
                bancoValido = true;
                break;
            }
        }

        if (!bancoValido) {
            cout << "Banco inválido.\n";
            return;
        }

        cin.ignore();
        cout << "Ingrese el número de cuenta: ";
        getline(cin, numeroCuenta);

        string query = "INSERT INTO cuentas_terceros(nombre, banco, numero_cuenta) VALUES('" + nombre + "', '" + bancoElegido + "', '" + numeroCuenta + "')";
        if (mysql_query(conectar, query.c_str()) == 0)
            cout << "Cuenta de tercero agregada correctamente.\n";
        else
            cout << "Error al agregar cuenta: " << mysql_error(conectar) << "\n";
    }

    void transferirDesdeCuenta() {
        int origenId, terceroId;
        double monto;

        Cuenta cuenta;
        cuenta.ver();

        cout << "\nIngrese el ID de la cuenta propia de origen: ";
        cin >> origenId;

        cout << "--- Cuentas de Terceros ---\n";
        mysql_query(conectar, "SELECT id, nombre, banco, numero_cuenta FROM cuentas_terceros");
        MYSQL_RES* resultado = mysql_store_result(conectar);
        MYSQL_ROW fila;

        while ((fila = mysql_fetch_row(resultado))) {
            cout << "ID: " << fila[0] << " | Nombre: " << fila[1] << " | Banco: " << fila[2] << " | Cuenta: " << fila[3] << "\n";
        }

        cout << "Ingrese el ID de la cuenta de tercero destino: ";
        cin >> terceroId;
        cout << "Ingrese el monto a transferir: Q";
        cin >> monto;

        string consulta = "SELECT saldo FROM cuentas WHERE id = " + to_string(origenId);
        mysql_query(conectar, consulta.c_str());
        resultado = mysql_store_result(conectar);
        fila = mysql_fetch_row(resultado);

        if (!fila) {
            cout << "Cuenta de origen no encontrada.\n";
            return;
        }

        double saldoOrigen = stod(fila[0]);
        if (saldoOrigen < monto) {
            cout << "Saldo insuficiente.\n";
            return;
        }

        string query = "UPDATE cuentas SET saldo = saldo - " + to_string(monto) + " WHERE id = " + to_string(origenId);
        if (mysql_query(conectar, query.c_str()) == 0) {
            string mov = "INSERT INTO movimientos(cuenta_id, tipo, descripcion, monto) VALUES(" + to_string(origenId) + ", 'Transferencia a Tercero', 'Transferencia a cuenta de tercero ID " + to_string(terceroId) + "', -" + to_string(monto) + ")";
            mysql_query(conectar, mov.c_str());

            cout << "Transferencia a tercero realizada exitosamente.\n";
        }
        else {
            cout << "Error al realizar transferencia: " << mysql_error(conectar) << "\n";
        }
    }
};

class Movimiento {
public:
    void verHistorial() {
        int cuentaId;
        cout << "\nIngrese el ID de la cuenta para ver historial: ";
        cin >> cuentaId;

        string query = "SELECT tipo, descripcion, monto, fecha FROM movimientos WHERE cuenta_id = " + to_string(cuentaId) + " ORDER BY fecha DESC";
        if (mysql_query(conectar, query.c_str()) == 0) {
            MYSQL_RES* resultado = mysql_store_result(conectar);
            MYSQL_ROW fila;

            cout << "\n--- Historial de Movimientos ---\n";
            while ((fila = mysql_fetch_row(resultado))) {
                cout << fila[3] << " | " << fila[0] << " | " << fila[1] << " | Q" << fila[2] << "\n";
            }
        }
        else {
            cout << "Error al obtener historial: " << mysql_error(conectar) << "\n";
        }
    }

    void verUltimos() {
        int cuentaId;
        cout << "\nIngrese el ID de la cuenta para ver últimos 5 movimientos: ";
        cin >> cuentaId;

        string query = "SELECT tipo, descripcion, monto, fecha FROM movimientos WHERE cuenta_id = " + to_string(cuentaId) + " ORDER BY fecha DESC LIMIT 5";
        if (mysql_query(conectar, query.c_str()) == 0) {
            MYSQL_RES* resultado = mysql_store_result(conectar);
            MYSQL_ROW fila;

            cout << "\n--- Últimos 5 Movimientos ---\n";
            while ((fila = mysql_fetch_row(resultado))) {
                cout << fila[3] << " | " << fila[0] << " | " << fila[1] << " | Q" << fila[2] << "\n";
            }
        }
        else {
            cout << "Error al obtener movimientos: " << mysql_error(conectar) << "\n";
        }
    }
};

int main() {
    conectar = mysql_init(0);
    conectar = mysql_real_connect(conectar, "localhost", "root", "Root852/*-", "sistema1", 3306, NULL, 0);

    if (!conectar) {
        cout << "Error al conectar a la base de datos: " << mysql_error(conectar) << "\n";
        return 1;
    }

    Cuenta* cuentaPtr = new Cuenta();
    CuentaTercero cuentaTercero;
    Movimiento movimiento;

    int opcion;
    do {
        cout << "\n--- Menu Principal ---\n";
        cout << "1. Crear cuenta propia\n";
        cout << "2. Ver mis cuentas\n";
        cout << "3. Ingresar saldo a una cuenta\n";
        cout << "4. Agregar cuenta de terceros\n";
        cout << "5. Transferir entre cuentas propias\n";
        cout << "6. Transferir a cuenta de tercero\n";
        cout << "7. Ver historial de movimientos\n";
        cout << "8. Ver últimos 5 movimientos\n";
        cout << "10. Retirar en cajero\n";
        cout << "9. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
        case 1: cuentaPtr->crear(); break;
        case 2: cuentaPtr->ver(); break;
        case 3: cuentaPtr->ingresarSaldo(); break;
        case 4: cuentaTercero.agregar(); break;
        case 5: cuentaPtr->transferirEntre(); break;
        case 6: cuentaTercero.transferirDesdeCuenta(); break;
        case 7: movimiento.verHistorial(); break;
        case 8: movimiento.verUltimos(); break;
        case 9: cout << "Saliendo del sistema...\n"; break;
        case 10: cuentaPtr->retirarEnCajero(); break;
        default: cout << "Opcion invalida.\n";
        }

    } while (opcion != 9);

    delete cuentaPtr;
    mysql_close(conectar);
    return 0;
}
