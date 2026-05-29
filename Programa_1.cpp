#include <iostream>
using namespace std;

int main(){
    int estudiantes;
    int calificaciones;
    int aprobados = 0;
    int caliMax = 0;
    int caliMin = 101;
    cout << "Ingrese el numero de estudiantes" << endl;
    cin >> estudiantes;
 
    for(int i = 1; i <= estudiantes; i++){
        cout<<"Ingrese la calificación del estudiante: " << i << endl;
        cin >> calificaciones;

        //Funcion encargada de contar cuantos estudiantes aprobaron
        if (60 <= calificaciones){
            aprobados++;
        };
        
        if (calificaciones > 100)
        {
            cout << "No puedes ingresar una calificación mayor a 100"<< endl;
            i--;
        }
        
        //Funcion que evalua la calificación mas alta
        if (caliMax < calificaciones){
            caliMax = calificaciones;
        }
        
        if (calificaciones < caliMin){
            caliMin = calificaciones;
        }
    }
    cout << "Estudiantes aprobados: " << aprobados << endl;
    cout << "Calificación mas alta: " << caliMax << endl;
    cout << "Calificación mas baja: " << caliMin << endl;
    return 0;
}