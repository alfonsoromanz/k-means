//
//  main.cpp
//  k-means
//
//  Created by Alfonso Roman on 7/7/15.
//  Copyright (c) 2015 Alfonso Roman. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <list>
#include <set>
#include <ctime>

using namespace std;


void error(string str) {
    cout << "\n\n ERROR: " << str << "\n\n\n" ;
    exit(-1);
}

int stringToNumber ( const string &Text, int defValue = 0 ) {
    stringstream ss;
    for ( string::const_iterator i=Text.begin(); i!=Text.end(); ++i )
        if ( isdigit(*i) || *i=='e' || *i=='-' || *i=='+' || *i=='.' )
            ss << *i;
    int result;
    return ss >> result ? result : defValue;
}



void inicializarVectores (int ** vectores, int n, int dim, list<int> valores) {
    /* Se arma la matriz de los puntos en base a la lista sacada del archivo */
    
    for (int i=0; i<n; i++)
        for (int j=0; j<dim; j++) {
            vectores[i][j] = valores.front();
            valores.pop_front();
        }
}

void inicializarClusters (int * clusterActual, int * clusterAnterior, int n) {
    /*inicialmente se considera que ningun punto pertenece a ningún cluster */
    
    for (int i=0; i<n; i++) {
        clusterActual[i] = -1;
        clusterAnterior[i] = -2;
    }
}

void generarCentroides (float ** centroides, int k, int dim, int ** vectores, int n) {
    /* se generan al azar los primeros centroides, tomados del dataset (k puntos al azar) */
    
    set<int> elegidos;
    for (int i=0; i<k; i++) {
        srand(time(0));
        int elegido = rand() % n;
        while (elegidos.find(elegido)!= elegidos.end()) {
            elegido = rand() % n; //para no elegir repetidos
        }
        elegidos.insert(elegido);
        for (int j=0; j<dim; j++)
            centroides[i][j] = vectores [elegido][j]; //se copia el centroide desde el dataset
    }
    
}

float distanciaEuclidiana (int ** vectores, float ** centroides, int p1, int p2, int dim) {
    float dist = 0;
    for (int i=0; i<dim; i++){
        float dif = vectores[p1][i]- centroides[p2][i];
        dist += dif * dif;
    }
    return sqrt(dist);
}

float distanciaChebyshev (int ** vectores, float ** centroides, int p1, int p2, int dim) {
    float maxDif = abs(vectores[p1][0] - centroides[p2][0]);
    for (int i=1; i<dim; i++) {
        float dif = abs(vectores[p1][i] - centroides[p2][i]);
        if (dif > maxDif)
            maxDif = dif;
    }
    return maxDif;
}


float distancia (int ** vectores, float ** centroides, int p1, int p2, int dim, int metrica) {
    switch (metrica) {
        case 1:
            return distanciaEuclidiana(vectores, centroides, p1, p2, dim);
            break;
            
       case 2:
            return distanciaChebyshev(vectores, centroides, p1, p2, dim);
            break;
            
        default:
            error ("Metrica incorrecta. ");
            return 0;
            break;
    }
}


bool converge (int * clusterActual, int * clusterAnterior, int n, int corridas) {
    /*if (corridas < 100)
        return false;*/
    for (int i=0; i<n; i++)
        if (clusterActual[i] != clusterAnterior[i])
            return false;
    return true;
}

void recalcularCentroides (float ** centroides, int ** vectores, int * clusterActual ,int k, int n, int dim) {
    
    float ** sumaCentroides; //tendra la suma de los vectores asociados a ese centroide
    sumaCentroides = new float * [k];
    for (int i=0; i<k; i++)
        sumaCentroides[i] = new float [dim];
    
    for (int i=0; i<k; i++)
        for (int j=0; j<dim; j++)
            sumaCentroides[i][j]=0;
    
    int * cantidadPuntos = new int [k]; //tendra la cantidad de puntos asociados a ese cluster
    for (int i=0; i<k; i++)
        cantidadPuntos[i] = 0;
    
    //se recalculan
    
    for (int i=0; i<n; i++) { //por cada punto
        int cluster = clusterActual[i];
        cantidadPuntos[cluster]++;
        for (int j=0; j<dim; j++) {
            sumaCentroides[cluster] [j] += vectores [i][j];
        }
    }
    
    
    //se copian los nuevos centroides. Metodo que evita los clusteres vacíos (promedia con el centroide anterior también)
    
    for (int i=0; i<k; i++) {
        if (cantidadPuntos[i]==0) cout << "CLUSTER VACIO: " << i << endl;
        for (int j=0; j<dim; j++)
            centroides[i][j] = (float)((centroides[i][j] + sumaCentroides[i][j]) / (cantidadPuntos[i]+1));
    }
    //Libero la memoria
    for (int i=0; i<k; i++)
        free(sumaCentroides[i]);
    free(sumaCentroides);
    free (cantidadPuntos);
}



/* ---- PROGRAMA PRINCIPAL ---- */

int main(int argc, const char * argv[]) {
    
    
    if (argc < 4)  {
        error ("Error. Verificar parametros pasados a la aplicacion");
    }
    

    ifstream dataset (argv[1]);
    
    if (!dataset.is_open()) {
        
       error("No se pudo abrir el archivo");
        
    } else {
        
        int dim; //contendrá la dimensión del espacio
        int n; //la cantidad de puntos a clusterizar
        int k; //contendra la cantidad de clusters
        int metrica = *argv[2] - 48; //parámetro que indica la metrica de distancia a utilizar
        char c;
        
        
        //Se calcula la cantidad de clusters k
        string m = argv[3];
        for (int i=4; i<argc; i++)
            m+=argv[i];
        
        k = stringToNumber(m);
            
        cout << "\n\nCantidad de Clusters: " << k << endl;
       
        
        //Se calcula la dimensión del espacio viendo el archivo. Se almacena en dim
        dim=0;
        bool finished = 0;
        while (dataset.get(c) && !finished) {
            while (c=='\b' || c=='\t'|| c== '\n')
                dataset.get(c);
            dim++;
            while (c!='\b' && c!='\t' &&c!= '\n')
                dataset.get(c);
            if (c == '\n')
                finished = 1;
        }
        
        dataset.seekg (0, ios::beg);
        
        cout << "Dimension calculada : " << dim << endl;
        
        
        //Se levantan los valores del dataset
        list<int> valores;
        string s="";
        
        while (dataset.get(c) && c!='\b' && c!='\t') {
            while (c=='\b' || c=='\t'|| c== '\n') {
                dataset.get(c);
            }
            s+=c;
            dataset.get(c);
            while (c!='\b' && c!='\t' &&c!= '\n') {
                s+=c;
                dataset.get(c);
            }
            int i = stringToNumber(s);
            s="";
            valores.push_back(i);
        }
        
        dataset.close();
        
        
        
        
        //Se crean las estructuras primarias
        
        n = (int)(valores.size()/dim); //se calcula la cantidad de puntos a clusterizar
        cout << "Cantidad de puntos : " << n << endl;
        
        int ** vectores; //contendrá todos los puntos del espacio (tamaño N x D)
        int * clusterActual; // en la posición "i" contiene el cluster asociado al punto "i"
        int * clusterAnterior; // analogo al anterior, usado para convergencia
        float ** centroides; //En la fila "i" se encuentra el centroide del cluster "i"
        
        
        //Creación e inicialización de las estructuras
        vectores = new int * [n];
        for (int i = 0; i<n; i++)
            vectores[i] = new int [dim];
        
        clusterActual = new int [n];
        
        clusterAnterior = new int [n];
        
        centroides = new float * [k];
        for (int i=0; i<k; i++)
            centroides[i] = new float [dim];
        
        inicializarVectores(vectores,n,dim, valores);
        inicializarClusters (clusterActual, clusterAnterior, n);
        generarCentroides (centroides, k, dim, vectores, n);
        
        cout << "Dataset: " << endl;
        for (int i=0; i<n; i++) {
            for (int j=0; j<dim; j++)
                cout << vectores [i][j] << " ";
            cout << endl;
        }
        
        cout << endl << "Centroides iniciales: " << endl;
        
        for (int i=0; i<k; i++) {
            for (int j=0; j<dim; j++)
                cout << centroides [i][j] << " ";
            cout << endl;
        }
        
        
        
        //COMIENZO DEL ALGORITMO K-MEANS
        int corridas = 0;
        
        cout << "\n\nComienzo de la clusterización :";
        
        auto begin = std::chrono::high_resolution_clock::now();
        
        while (!converge(clusterActual, clusterAnterior, n, corridas)) {
            corridas++;
            cout << "\nIteración: " << corridas << endl; //Para testing
            
            //Se asignan los puntos a un cluster, basandose en el más cercano
            for (int i=0; i<n; i++) {
                clusterAnterior [i] = clusterActual [i]; //se salva el anterior
                int masCercano = 0;
                float distMenor = distancia(vectores, centroides, i, 0, dim, metrica);
                for (int j=1; j<k; j++){
                    //por cada centroide
                    float dist = distancia(vectores, centroides, i, j, dim, metrica);
                    if (dist < distMenor) {
                        distMenor = dist;
                        masCercano = j;
                    }
                }
                clusterActual[i] = masCercano; //reasignación del centroide
            }
        
            recalcularCentroides(centroides, vectores, clusterActual, k, n, dim);
            cout << endl << endl << "Centroides recalculados: " << endl;
            
            //Para testing
            for (int i=0; i<k; i++) {
                for (int j=0; j<dim; j++)
                    cout << centroides [i][j] << " ";
                cout << endl;
            }
        }
        
        //FIN DE LA CLUSTERIZACIÓN
        
        
        auto end = std::chrono::high_resolution_clock::now();
        
        cout << "\n\nFin de la clusterización. Tiempo Transcurrido: ";
        cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << " ns" << std::endl;
        
        int * cantidadPuntos = new int [k]; //tendra la cantidad de puntos asociados a ese cluster
        for (int i=0; i<k; i++)
            cantidadPuntos[i] = 0;
        
        
        //Se muestra el resultado
        cout << endl << endl << "Resultado: \n";
        for (int i=0; i<n; i++) {
            cout << clusterActual [i] << endl;
            cantidadPuntos[clusterActual[i]]++;
        }
        
        
        //Guardo el resultado
        ofstream salida;
        
        string ruta = argv[1];
        while (ruta[ruta.length()-1]!= '/')
            ruta.pop_back();
        ruta.append("resultado.txt");
        salida.open (ruta);
        salida << "Resultados de la clasificación:\n";
        salida << "********** ** ** **************\n\n";
        salida << "Dataset : " << argv[1] << "\n";
        salida << "Dimension: " << dim << "\n";
        salida << "Cantidad de puntos: " << n << "\n";
        salida << "Cantidad de clusters: " << k << "\n";
        salida << "Métrica utilizada: " << (metrica == 1? "Euclidiana" : "Chebyshev") << "\n";
        salida << "Cantidad de iteraciones: " << corridas << "\n";
        salida << "Tiempo de clusterización: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << " ns\n\n";
        for (int i=0; i<n; i++) {
            salida << clusterActual [i] << endl;
        }
        salida << "\nCantidad de puntos asociados a cada cluster (cluster : cantidad) : \n";
        for (int i=0; i<k; i++) {
            salida << " Cluster " << i << " : " << cantidadPuntos[i] << endl;
        }
        salida.close();
        
        cout << endl << "Se ha guardado un informe del resultado en: " << ruta << endl << endl;
        
        
        
    }
    
    return 0;
}