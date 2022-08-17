#ifndef Btree_H
#define Btree_H

#include <iostream>
#include "Queue.h"
using namespace std;

class Btree{
    private:
        Node* raiz;         //Indica el Node de la raiz del Btree.
        int orden;          //Indica la cantidad de valores maximo de los Nodes hoja.
        bool encontrado;    //Indica si se encontro un valor en el Btree.
        Node* ubication;        //Indica ubication se encontro el valor que se busco.

    public:
        Btree(int a);
        ~Btree();

        //Pemite find un valor en el Btree o la ubicacion ubication va a agregarse.
        int find(int a);

        //Este metodo permite agregar un valor al Btree y ajustarlo en caso de ser necesario.
        void insert(int a);
        //Este metodo auxiliar del agregar permite dividir los Nodes hojas en caso de llenarse el Node.
        void splitSheets(int a, Node *p);
        //Este metodo auxiliar del agregar permite dividir los Nodes intermedios para ajustar el Btree.
        void splitPatern(Node *p);

        //Este metodo permite erase un valor al Btree y ajustarlo en caso de ser necesario.
        int erase(int a);
        //Este metodo auxiliar del erase permite ajustar los Nodes hojas en caso de tener menos valores de los minimos permitidos.
        void modifySheets(int a, Node *p);
        //Este metodo auxiliar del erase permite ajustar los Nodes intermedios para ajustar el Btree cuando el Node tenga menos direcciones de las permitidas.
        void modifyPatern(Node *p);

        //Este metodo permite leer un archivo con los valores que se agregaran al Btree.
        void readKeys(string mensaje);
        //Este metodo permite leer un archivo con los valores que se erasean del Btree.
        void eraseKeys(string mensaje);

        //Este metodo permite draw los Nodes del Btree con todos los valores que contenga.
        void draw();

};



Btree::Btree(int a){
    orden = a;
    raiz = NULL;
    encontrado = false;
}

Btree::~Btree(){
    Node *p;
    int minimo;
    while(raiz  != NULL){//Busca el mayor valor en el Btree y lo borra del Btree mientras exista algo en el Btree
        p = raiz;
        while(p->esHoja == false){
            p = p->direccion.ubicationPrincipio()->direccion;
        }
        minimo = p->llaves.ubicationPrincipio()->value;
        erase(minimo);
    }
    raiz = NULL;
    orden = 0;
    encontrado = false;

}

/*
    Busca la ubicacion para agregar el nuevo valor, si esta repetido no se agrega.
*/

int Btree::find(int a){
    Node *p = raiz;
    cashValue *val;
    cashDirection *q;

    int n = 0;
    while(true){ //Busca la hoja del valor
        if(p->esHoja == true){ //Busca si esta agregado el elemento en esa hoja

            val = p->llaves.ubicationPrincipio();

            while(val  != NULL){
                if(val->value == a){
                    encontrado = true;
                    ubication = p;
                    return 0;
                }
                val = val->next;

            }

            encontrado = false;
            ubication = p;
            return 1;

        }else{ //Busca en que direccion del Node del Btree debe bajar

            n = 0;
            val = p->llaves.ubicationPrincipio();
            q = p->direccion.ubicationPrincipio();

            while(q  != NULL && val  != NULL){
                    if(a>=val->value){
                        n++;

                        val = val->next;

                        q = q->next;
                    }else{
                        val = NULL;
                    }

            }
            p = q->direccion;
        }

    }


}

/*
    Permite insert un valor al Btree y ajustar el Btree en caso de ser necesario.
*/
void Btree::insert(int a){
    Node *p;
    if(raiz == NULL){//Si el Btree no contiene elementos

        p =  new Node;
        p->llaves._new();
        p->direccion._new();
        p->esHoja = true;
        p->padre = NULL;

        p->llaves.insert(a);
        raiz = p;
        return;

    }else{//Si el Btree contiene elementos y se debe find ubication se agregara el valor

        find(a);

        if(encontrado == true){
            return;
        }

        p = ubication;

        if(p->llaves.CuantosVal()<orden-1){ //Si despues de agregar no se debe modificar los Nodes del Btree
            p->llaves.insert(a);
        }else{ //Si al agregar el valor se debera reajustar el Btree
            splitSheets(a, p);
        }
        return;

    }

}

/*
    Permite dividir las hojas del Btree y en caso de ser necesarios reajustar los Nodes padres del Btree.
*/
void Btree::splitSheets(int a, Node *p){

    p->llaves.insert(a);

    int medio = orden/2, val;
    if(p == raiz){ //Si la raiz es una hoja y debe dividirse por estar lleno el Node

        //Crear padre y hermano derecho
        Node* padre = new Node;
        padre->llaves._new();
        padre->direccion._new();
        padre->esHoja = false;
        padre->padre = NULL;

        Node* hijo2 = new Node;
        hijo2->llaves._new();
        hijo2->direccion._new();
        hijo2->esHoja = true;
        hijo2->padre=padre;

        Node *hijo1 = p;


        //pasar valores del Node al hermano derecho
        for(int i=medio; i<orden; i++){
            val = hijo1->llaves.take();

            hijo2->llaves.insert(val);

        }

        //Agregar direcciones y valor al Node padre
        padre->llaves.insert(hijo2->llaves.ubicationPrincipio()->value);

        padre->direccion.insert(hijo1);

        padre->direccion.insert(hijo2);

        //asignar nueva raiz
        raiz = padre;

        //asignar padres
        hijo1->padre = padre;

        //Asignar nuevas hojas
        hijo1->esHoja = true;


    }else if((p->padre)->llaves.CuantosVal()<orden-1 && p->esHoja == true){//Si el padre del Node le cabe otra direccion

        //Crear hermano derecho
        Node* hijo2 = new Node;
        hijo2->llaves._new();
        hijo2->direccion._new();
        hijo2->esHoja = true;
        hijo2->padre = p->padre;

        //enviar valores al hermano derecho
        for(int i = medio; i<orden; i++){
            val = p->llaves.take();

            hijo2->llaves.insert(val);

        }

        //Agrega una direccion y una llave al hermano derecho
        (p->padre)->llaves.insert(hijo2->llaves.ubicationPrincipio()->value);

        (p->padre)->direccion.insert(hijo2);

    }else{//Si el padre se llenara y debera separarse en Nodes intermedios

        //Crea nuevo hijo
        Node* hijo2 = new Node;
        hijo2->llaves._new();
        hijo2->direccion._new();
        hijo2->padre = p->padre;
        hijo2->esHoja = true;

        //Agrega los valores al hijo nuevo
        for(int i = medio; i<orden; i++){
            val = p->llaves.take();

            hijo2->llaves.insert(val);

        }
        //Asigna las propiedades del hijo

        (p->padre)->llaves.insert(hijo2->llaves.ubicationPrincipio()->value);

        (p->padre)->direccion.insert(hijo2);



        Queue Q;
        Node *next, *q;
        q = p->padre;
        int sigue = 1;
        //Se agregan todos los Nodes intermedios que se han llenado a una Queue
        if(q->llaves.CuantosVal() == orden){
            Q.insert(q);
            q = q->padre;
            if(q  != NULL){
                while(q  != raiz  && sigue == 1){
                    if(q->llaves.CuantosVal() == (orden-1) ){
                        Q.insert(q);
                        q = q->padre;
                    }else{
                        sigue = 0;
                    }
                }
                if(sigue == 1){
                    if(q->llaves.CuantosVal() == orden-1){
                        Q.insert(q);
                    }
                }
            }
            //Todos los Nodes de la Queue se deberan separar
            while(!Q.estaVacia()){
                next = Q.take();
                splitPatern(next);
            }

        }
    }

}

/*
    Permite dividir los Nodes intermedios y reajustar el Btree.
    Node *p: la direccion del Node que se debera ajustar en el Btree.
*/
void Btree::splitPatern(Node *p){

    int medio = orden/2;
    int valM, val;
    Node *direccion;


    Node* hijo2 = new Node;
    hijo2->llaves._new();
    hijo2->direccion._new();
    hijo2->esHoja = false;
    hijo2->padre = NULL;

    //Agregar valores hijo derecho
    for(int i = medio+1; i<orden; i++){

        val = p->llaves.take();

        hijo2->llaves.insert(val);

    }
    //Sacar el valor central
    valM = p->llaves.take();

    //Agregar direcciones hijo derecho
    for(int i = medio+1; i<orden+1; i++){
        direccion = p->direccion.take();
        hijo2->direccion.insert(direccion);
        //asigno nuevo padre
        direccion->padre = hijo2;
    }

    if(p  != raiz){
        if(p->esHoja == true){
            val = hijo2->llaves.ubicationPrincipio()->value;
            (p->padre)->llaves.insert(val);
            (p->padre)->direccion.insert(hijo2);
            hijo2->padre = p->padre;
        }else{
            (p->padre)->llaves.insert(valM);
            (p->padre)->direccion.insert(hijo2);
            hijo2->padre = p->padre;
        }
    }else{
        Node* padre = new Node;
        padre->llaves._new();
        padre->direccion._new();
        padre->esHoja = false;
        padre->padre = NULL;

        padre->llaves.insert(valM);
        padre->direccion.insert(p);
        padre->direccion.insert(hijo2);
        raiz = padre;
        p->padre = padre;
        hijo2->padre = padre;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Recibe un valor que debera ser borrado del Btree.
*/
int Btree::erase(int a){
    Node *p;
    find(a);

    if(encontrado == false){//Si no se encuentra no se hace nada
        return 0;
    }
    p = ubication;//La ubicacion ubication se encontro el valor
    if(p == raiz){//Si solo se tiene la raiz
        if(p->llaves.CuantosVal() == 1){//Si la raiz solo tiene un elemento se borra y se deja el Btree como al principio
            p->direccion.empty();
            p->llaves.empty();
            p->esHoja = false;
            p->padre = NULL;
            delete p;
            raiz = NULL;
            return 1;
        }else{//Si la raiz tiene mas de un elemento solo se borra el numero
            p->llaves.erase(a);
            return 1;
        }

    }else{//Si el Btree tiene mas de un Node

        if(p->llaves.CuantosVal()>(int)orden/2){//Si la hoja tiene mas llaves que las minimas requeridas
            if(a  != p->llaves.ubicationPrincipio()->value || (p->padre)->direccion.ubicationPrincipio()->direccion == p){//Si no es el primer elemento del Node o cuelga como primer hijo solo se saca la hoja
                p->llaves.erase(a);
            }else{// En este caso se debe sacar el menor valor en el Node en el Node padre y agregar el que queda en primera posicion
                p->llaves.erase(a);
                (p->padre)->llaves.erase(a);
                (p->padre)->llaves.insert(p->llaves.ubicationPrincipio()->value);
            }
            return 1;

        }else{//Si el Node tiene menos llaves de las requeridas y es necesario reorganizar el Btree
            modifySheets(a, p);
        }
    }
    return 1;
}

/*
    Modifica las hojas del Btree y ajusta el Btree de ser necesario.
*/

void Btree::modifySheets(int a, Node *p){
    Node *h_der, *h_izq, *padre, *aux;
    int indice, valor;
    cashDirection *q, *i;
    cashValue *v;
    //Se saca el valor de la hoja
    p->llaves.erase(a);

    padre = p->padre;
    indice = 0;
    //Se busca en que indice del padre esta colgado esa hoja
    q = padre->direccion.ubicationPrincipio();
    while(q->direccion  != p){
        indice+=1;
        if(q->next  != NULL){
            if(q->next->direccion == p){
                i = q;
            }
        }
        q = q->next;
    }

    if(indice == 0){// Si cuelga en la posicion 0 del padre

        //Debe tener hermano derecho
        h_der = q->next->direccion;

        if(h_der->llaves.CuantosVal()>(int)orden/2){   //CASO 1           //Se revisa si el hermano derecho puede prestar un valor
            //Se saca la primera llave del hermano derecho y se agrega al Node actual
            valor = h_der->llaves.ubicationPrincipio()->value;
            h_der->llaves.erase(valor);
            p->llaves.insert(valor);

            //Se cambia la llave en el padre que indica el primer elemento del hermano derecho(ya que se presto el primer valor del hermano derecho)
            padre->llaves.ubicationPrincipio()->value = h_der->llaves.ubicationPrincipio()->value;

        }else{      //CASO 2         //Si el hermano derecho no le puede prestar, se fucionan en el actual y se limpia el hermano derecho.
            //se saca la primera llave del padre ya que el Node hermano derecho ya no existira
            padre->llaves.erase(padre->llaves.ubicationPrincipio()->value);

            //Se transfieren las llaves del hermano derecho al Node actual, ya que al ser hoja no tiene direcciones
            while(h_der->llaves.CuantosVal()>0){
                valor = h_der->llaves.ubicationPrincipio()->value;
                h_der->llaves.erase(valor);
                p->llaves.insert(valor);
            }
            //Se desconecta el hermano derecho del padre.
            padre->direccion.erase(h_der);

            //Se libera memoria
            h_der->direccion.empty();
            h_der->llaves.empty();
            h_der->esHoja = false;
            h_der->padre = NULL;
            delete h_der;

        }

    }else if(indice<orden-1){//Si el Node cuelga entre la llaves[1] y llaves[orden-1]

        //Debe tener hermano izquierdo
        h_izq = i->direccion;

        v = padre->llaves.ubicationPrincipio();
        for(int i = 1; i<indice; i++){
            v = v->next;
        }
        if(q->next  != NULL){   //checar si tiene hermano derecho

            h_der = q->next->direccion;

            if(h_der->llaves.CuantosVal()>(int)orden/2||h_izq->llaves.CuantosVal()>(int)orden/2){//Se revisa si alguno de sus hermanos le puede prestar una llave

                if(h_der->llaves.CuantosVal()>(int)orden/2){       //CASO 3    //Se revisa primero el hermano derecho(si los dos le pueden prestar se usara el hermano derecho)
                    //Se saca la primera llave del hermano derecho y se agrega al Node actual
                    valor = h_der->llaves.ubicationPrincipio()->value;
                    h_der->llaves.erase(valor);
                    p->llaves.insert(valor);

                    //Si se borro el primer elemento del Node actual actualizar el indice en el padre

                    if(v->value  != p->llaves.ubicationPrincipio()->value){
                        v->value = p->llaves.ubicationPrincipio()->value;
                    }

                    //Se cambia la llave en el padre que indica el primer elemento del hermano derecho(ya que se presto el primer valor del hermano derecho)
                    (v->next)->value = h_der->llaves.ubicationPrincipio()->value;

                }else{      //CASO 4     //Solo el hermano izquierdo es viable para prestarle una llave
                    //Se pide la ultima llave del hermano izquierdo para agregar al actual
                    valor = h_izq->llaves.take();
                    h_izq->llaves.erase(valor);
                    p->llaves.insert(valor);

                    //Se cambia la llave en el padre que indica el primer elemento del Node actual(Ya que al pedir prestado al hermano izquierdo, la llave nueva sera la menor del Node actual)
                    v->value = valor;

                }
            }else{    //CASO 5      //Como no se puede pedir a los hermanos, las llaves del hermano derecho se unen al Node actual
                //Se saca el indice de la primera llave del hermano derecho en el Node padre
                valor = (v->next)->value;
                padre->llaves.erase(valor);
                //Se saca la direccion del hermano derecho en el padre
                padre->direccion.erase(h_der);

                //Se sacan las llaves del hermano derecho para agregarse al Node actual
                while(h_der->llaves.CuantosVal()>0){
                    valor = h_der->llaves.ubicationPrincipio()->value;
                    h_der->llaves.erase(valor);
                    p->llaves.insert(valor);
                }
                //Si se borro el primer elemento del Node actual actualizar el indice en el padre
                if(v->value  != p->llaves.ubicationPrincipio()->value){
                        v->value = p->llaves.ubicationPrincipio()->value;
                }

                //Liberar memoria del hermano derecho
                h_der->direccion.empty();
                h_der->llaves.empty();
                h_der->esHoja = false;
                h_der->padre = NULL;
                delete h_der;

            }
        }else{//Solo tiene hermano izquierdo

            if(h_izq->llaves.CuantosVal()>(int)orden/2){       //CASO 6           //Pedir al hermano izquierdo en caso de poder
                //Se saca la ultima llave del hermano izquierdo y se agrega al actual Node
                valor = h_izq->llaves.take();
                h_izq->llaves.erase(valor);
                p->llaves.insert(valor);

                //Se indica la nueva primera llave del Node en el padre
                v->value = p->llaves.ubicationPrincipio()->value;

            }else{      //CASO 7        //mandar llaves al hijo izquierdo y limpiar Node
                //Sacar indice y direccion del Node actual en el padre
                //Solo tiene hermano izquierdo
                padre->llaves.take();
                padre->direccion.take();

                //Mandar llaves del Node actual al hermano izquierdo

                while(p->llaves.CuantosVal()>0){
                    valor = p->llaves.ubicationPrincipio()->value;
                    p->llaves.erase(valor);
                    h_izq->llaves.insert(valor);
                }
                //Liberar memoria del Node actual
                p->direccion.empty();
                p->llaves.empty();
                p->esHoja = false;
                p->padre = NULL;
                delete p;

            }
        }
    }else{//Es la ultima direccion del Node (Solo tiene hermano izquierdo)

        h_izq = i->direccion;

        v = padre->llaves.ubicationPrincipio();
        for(int i = 1; i<indice; i++){
            v = v->next;
        }

        if(h_izq->llaves.CuantosVal()>(int)orden/2){       //CASO 8        //pedir prestado al hermano izquierdo
            //Se saca la ultima llave del hermano izquierdo y se agrega al actual Node
            valor = h_izq->llaves.take();

            h_izq->llaves.erase(valor);
            p->llaves.insert(valor);

            //Se indica la nueva primera llave del Node en el padre(ya que el hermano izquierdo le presto una llave menor que las que contenia)
            v->value = p->llaves.ubicationPrincipio()->value;

        }else{              //CASO 9        //mandar llaves al hermano izquierdo y limpiar Node
            //Sacar indice y direccion del Node actual en el padre
            valor = v->value;
            padre->llaves.take();

            //Es la ultima direccion del Node padre
            padre->direccion.take();

            //Mandar llaves del Node actual al hermano izquierdo
            while(p->llaves.CuantosVal()>0){
                valor = p->llaves.ubicationPrincipio()->value;
                p->llaves.erase(valor);
                h_izq->llaves.insert(valor);
            }
            //Liberar memoria del Node actual
            p->direccion.empty();
            p->llaves.empty();
            p->esHoja = false;
            p->padre = NULL;
            delete p;

        }

    }

    //Falta checar cuando los padres se veran afectados
    while(padre  != raiz){

            //Se revisa si el padre tiene menos direcciones de las minimas para cumplir las reglas del Btree
            if(padre->direccion.CuantosDir()<(orden-1)/2+1){

                    //Si es necesario ajustar el Btree se ajusta y se revisa el padre del Node actual
                    aux = padre->padre;//Se guarda el padre por si acaso se libera memoria al modificar los padres
                    modifyPatern(padre);
                    padre = aux;

            }else{
                //En caso de no ser necesario ajustar el Btree
                padre = raiz;
            }

    }

    //Se revisa si la raiz se debe ajustar
    if(raiz->direccion.CuantosDir() < 2){

        p = raiz;
        raiz = raiz->direccion.ubicationPrincipio()->direccion;
        raiz->padre = NULL;
        p->direccion.empty();
        p->llaves.empty();
        p->esHoja = false;
        p->padre = NULL;
        delete p;

    }
}

/*
    Modifica los Nodes intermedios del Btree y ajusta el Btree de ser necesario.
*/
void Btree::modifyPatern(Node *p){//Estos casos permiten ajustar el Btree en los Nodes intermedios
    Node *padre, *h_izq, *h_der, *direccion;
    int valor, indice = 0;
    padre = p->padre;
    cashDirection *q, *i;
    cashValue *v;

    q = padre->direccion.ubicationPrincipio();
    while(q->direccion  != p){
        indice+=1;
        if(q->next  != NULL){
                i = q;
        }
        q = q->next;

    }

    v = padre->llaves.ubicationPrincipio();
    for(int i = 1; i<indice; i++){
        v = v->next;
    }
    //Se saca el valor en el Node padre y se agrega al Node hijo 'p'
    valor = v->value;

    padre->llaves.erase(valor);
    p->llaves.insert(valor);

    if(indice == 0){//Es la primera direccion del Node padre

        //Debe tener hermano derecho ya que no puede tener una sola direccion

        h_der = (q->next)->direccion;

        if(h_der->llaves.CuantosVal()>(orden-1)/2){   //CASO 1   //El hermano derecho le puede prestar una llave

            //Se saca la primera llave del Node derecho y se agrega al Node padre
            valor = h_der->llaves.ubicationPrincipio()->value;
            h_der->llaves.erase(valor);
            padre->llaves.insert(valor);

            //Se saca la primera direccion del hermano derecho y se agrega a las direcciones del Node actual(quedara al final por ser una direccion del hermano derecho).
            direccion = h_der->direccion.ubicationPrincipio()->direccion;
            h_der->direccion.erase(direccion);
            p->direccion.insert(direccion);

            //Asignar nuevo padre al Node que se movio
            direccion->padre = p;

        }else{  //CASO 2  //El hermano derecho no le puede prestar
            //Se sacan las llaves del hermano derecho para agregarse al Node actual
            while(h_der->llaves.CuantosVal()>0){
                //valor = h_der->llaves.ubicationPrincipio()->value;
                //h_der->llaves.erase(valor);
                valor = h_der->llaves.take();

                p->llaves.insert(valor);

            }
            //Se sacan las direcciones del hermano derecho para agregarse al Node actual
            while(h_der->direccion.CuantosDir()>0){

                direccion = h_der->direccion.take();

                p->direccion.insert(direccion);

                //Se asignan sus nuevos padres a los Nodes
                direccion->padre = p;


            }
            //Se saca la direccion del hermano derecho en el padre
            padre->direccion.erase(h_der);


            //Liberar memoria del hermano derecho
            h_der->direccion.empty();
            h_der->llaves.empty();
            h_der->esHoja = false;
            h_der->padre = NULL;
            delete h_der;

        }

        //Si es el padre de una hoja se actualiza al primer valor de su nuevo
        if(p->direccion.ubicationPrincipio()->direccion->esHoja == true){
            v = p->llaves.ubicationPrincipio();
            for(int j = 0; j<((orden-1)/2-1); j++){
                v = v->next;
            }
            i = p->direccion.ubicationPrincipio();
            for(int j = 0; j<((orden+1)/2-1); j++){
                i = i->next;
            }
            v->value = i->direccion->llaves.ubicationPrincipio()->value;
        }

    }else if(indice<orden-1){//Si el Node cuelga entre la direccion[1] y direccion[orden-1]

        //Debe tener un hermano izquierdo
        h_izq = i->direccion;

        if(q->next  != NULL){//Checar si tiene hermano derecho

            h_der = q->next->direccion;

            if(h_der->llaves.CuantosVal()>(orden-1)/2||h_izq->llaves.CuantosVal()>(orden-1)/2){//Se revisa si el hermano derecho o izquierdo pueden prestar llaves sin incumplir las reglas del Btree

                if(h_der->llaves.CuantosVal()>(orden-1)/2){           //CASO 3        //Se revisa primero el hermano derecho para prestar
                    padre->llaves.insert(valor);

                    p->llaves.erase(valor);

                    v = padre->llaves.ubicationPrincipio();
                    for(int j = 1; j<indice+1; j++){
                        v = v->next;
                    }
                    valor = v->value;

                    p->llaves.insert(valor);

                    padre->llaves.erase(valor);


                    //Se saca la primera llave del hermano derecho y se agrega al Node padre
                    valor = h_der->llaves.ubicationPrincipio()->value;
                    h_der->llaves.erase(valor);

                    padre->llaves.insert(valor);


                    //Se saca la primera direccion del hermano derecho y se agrega a las direcciones del Node actual(quedara al final por ser una direccion del hermano derecho).
                    direccion = h_der->direccion.ubicationPrincipio()->direccion;
                    h_der->direccion.erase(direccion);
                    p->direccion.insert(direccion);

                    //Asignar nuevo padre al Node que se movio
                    direccion->padre = p;

                    //Si es el padre de una hoja se actualiza al primer valor de su nuevo
                    if(p->direccion.ubicationPrincipio()->direccion->esHoja == true){
                        v = p->llaves.ubicationPrincipio();
                        for(int j = 0; j<((orden-1)/2-1); j++){
                            v = v->next;
                        }
                        i = p->direccion.ubicationPrincipio();
                        for(int j = 0; j<((orden+1)/2-1); j++){
                            i = i->next;
                        }
                        v->value = i->direccion->llaves.ubicationPrincipio()->value;

                    }

                }else{    //CASO 4    //Solo el hermano izquierdo es viable para prestarle una llave
                    //Se saca la ultima llave del hermano izquierdo y se agrega al Node padre
                    valor = h_izq->llaves.take();
                    h_izq->llaves.erase(valor);
                    padre->llaves.insert(valor);

                    //Se saca la primera direccion del hermano izquierdo y se agrega a las direcciones del Node actual(quedara al principio por ser una direccion del hermano izquierdo).
                    direccion = h_izq->direccion.take();
                    h_izq->direccion.erase(direccion);
                    p->direccion.insert(direccion);

                    //Asignar nuevo padre al Node que se movio
                    direccion->padre = p;

                    //Si es el padre de una hoja se actualiza al primer valor de su nuevo
                    if(p->direccion.ubicationPrincipio()->direccion->esHoja == true){
                        i = p->direccion.ubicationPrincipio()->next;
                        p->llaves.ubicationPrincipio()->value = i->direccion->llaves.ubicationPrincipio()->value;

                    }

                }

            }else{    //CASO 5    //Ninguno de los dos hermanos pueden prestarle llaves y direcciones.
                while(p->llaves.CuantosVal()>0){
                    valor = p->llaves.ubicationPrincipio()->value;
                    p->llaves.erase(valor);
                    h_izq->llaves.insert(valor);
                }

                //Se sacan las direcciones del Node actual para agregarse al hermano izquierdo
                while(p->direccion.CuantosDir()>0){
                    direccion = p->direccion.ubicationPrincipio()->direccion;

                    h_izq->direccion.insert(direccion);

                    p->direccion.erase(direccion);

                    //Se asignan sus nuevos padres a los Nodes
                    direccion->padre = h_izq;

                }

                padre->direccion.erase(p);

                //Liberar memoria del Node actual
                p->direccion.empty();
                p->llaves.empty();
                p->esHoja = false;
                p->padre = NULL;
                delete p;

                //Si es el padre de una hoja se actualiza al primer valor de su nuevo
                if(h_izq->direccion.ubicationPrincipio()->direccion->esHoja == true){
                    v = h_izq->llaves.ubicationPrincipio();
                    for(int j = 0; j<((orden-1)/2); j++){
                        v = v->next;
                    }

                    i = h_izq->direccion.ubicationPrincipio();
                    for(int j = 0; j<((orden+1)/2); j++){
                        i = i->next;
                    }

                    v->value = i->direccion->llaves.ubicationPrincipio()->value;
                }

            }

        }else{//Solo tiene hermano izquierdo
            if(h_izq->llaves.CuantosVal()>(orden-1)/2){    //CASO 6    //Pedir al hermano izquierdo en caso de poder
                //Se saca la ultima llave del hermano izquierdo y se agrega al Node padre
                valor = h_izq->llaves.take();
                h_izq->llaves.erase(valor);
                padre->llaves.insert(valor);

                //Se saca la primera direccion del hermano izquierdo y se agrega a las direcciones del Node actual(quedara al principio por ser una direccion del hermano izquierdo).
                direccion = h_izq->direccion.take();
                h_izq->direccion.erase(direccion);
                p->direccion.insert(direccion);

                //Asignar nuevo padre al Node que se movio
                direccion->padre = p;

                //Si es el padre de una hoja se actualiza al primer valor de su nuevo
                if(p->direccion.ubicationPrincipio()->direccion->esHoja == true){
                    i = p->direccion.ubicationPrincipio()->next;
                    p->llaves.ubicationPrincipio()->value = i->direccion->llaves.ubicationPrincipio()->value;

                }

            }else{        //Caso 7    //Se tiene que unir con el hermano izquierdo
                //Se sacan las llaves del Node actual para agregarse al hermano izquierdo
                while(p->llaves.CuantosVal()>0){
                    valor = p->llaves.ubicationPrincipio()->value;
                    p->llaves.erase(valor);
                    h_izq->llaves.insert(valor);
                }

                //Se sacan las direcciones del Node actual para agregarse al hermano izquierdo
                while(p->direccion.CuantosDir()>0){
                    direccion = p->direccion.ubicationPrincipio()->direccion;

                    h_izq->direccion.insert(direccion);

                    p->direccion.erase(direccion);

                    //Se asignan sus nuevos padres a los Nodes
                    direccion->padre = h_izq;

                }

                //Se saca la direccion del Node actual en el padre
                padre->direccion.erase(p);

                //Liberar memoria del Node actual
                p->direccion.empty();
                p->llaves.empty();
                p->esHoja = false;
                p->padre = NULL;
                delete p;

                //Si es el padre de una hoja se actualiza al primer valor de su nuevo
                if(h_izq->direccion.ubicationPrincipio()->direccion->esHoja == true){
                    v = h_izq->llaves.ubicationPrincipio();
                    for(int j = 0; j<((orden-1)/2); j++){
                        v = v->next;
                    }

                    i = h_izq->direccion.ubicationPrincipio();
                    for(int j = 0; j<((orden+1)/2); j++){
                        i = i->next;
                    }

                    v->value = i->direccion->llaves.ubicationPrincipio()->value;

                }

            }

        }
    }else{//Solo tiene hermano izquierdo

        h_izq = i->direccion;

        if(h_izq->llaves.CuantosVal()>(orden-1)/2){    //Caso 8    //El hermano izquierdo le puede prestar una llave y una direccion
            //Se saca la ultima llave del hermano izquierdo y se agrega al Node padre
            valor = h_izq->llaves.take();
            h_izq->llaves.erase(valor);
            padre->llaves.insert(valor);

            //Se saca la primera direccion del hermano izquierdo y se agrega a las direcciones del Node actual(quedara al principio por ser una direccion del hermano izquierdo).
            direccion = h_izq->direccion.take();
            h_izq->direccion.erase(direccion);
            p->direccion.insert(direccion);

            //Asignar nuevo padre al Node que se movio
            direccion->padre = p;


            //Si es el padre de una hoja se actualiza al primer valor de su nuevo
            if(p->direccion.ubicationPrincipio()->direccion->esHoja == true){
                i = p->direccion.ubicationPrincipio()->next;
                p->llaves.ubicationPrincipio()->value = i->direccion->llaves.ubicationPrincipio()->value;

            }

        }else{    //Caso 9    //Se tiene que unir el Node con su hermano izquierdo
            //Se sacan las llaves del Node actual para agregarse al hermano izquierdo
            while(p->llaves.CuantosVal()>0){
                valor = p->llaves.take();

                h_izq->llaves.insert(valor);
            }

            //Se sacan las direcciones del Node actual para agregarse al hermano izquierdo
            while(p->direccion.CuantosDir()>0){
                direccion = p->direccion.take();
                h_izq->direccion.insert(direccion);
                //Se asignan sus nuevos padres a los Nodes
                direccion->padre = h_izq;
            }

            //Se saca la direccion del Node actual en el padre
            padre->direccion.erase(p);
            //Liberar memoria del Node actual
            p->direccion.empty();
            p->llaves.empty();
            p->esHoja = false;
            p->padre = NULL;
            delete p;

            //Si es el padre de una hoja se actualiza al primer valor de su nuevo
            if(h_izq->direccion.ubicationPrincipio()->direccion->esHoja == true){
                v = h_izq->llaves.ubicationPrincipio();
                for(int j = 0; j<((orden-1)/2); j++){
                    v = v->next;
                }
                i = h_izq->direccion.ubicationPrincipio();
                for(int j = 0; j<((orden+1)/2); j++){
                    i = i->next;
                }
                v->value = i->direccion->llaves.ubicationPrincipio()->value;

            }

        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Permite leer un archivo ubication se encuentran los valores que se agregaran al Btree.
*/
void Btree::readKeys(string mensaje){
    //Se abre el archivo
    string ruta = "casos/"+mensaje+".txt";
    char *y = (char *)ruta.c_str();
    ifstream fin(y);
    //En caso de no poder abrir el archivo se le avisa al usuario
    if(fin.fail()){
        cout<<"No se pudo abrir el archivo";
        system("pause");
        exit(1);
    }
    int arcos, a;
    //Se lee la cantidad de numeros que se leeran
    fin>>arcos;
    for(int i=1; i<=arcos; i++){
	//Se agrega la informacion a la grafica
        fin>>a;
        insert(a);
        cout<<"\n -> Numero agregado: "<<a<<"\n";
    }
    cout<<"\n\n\n";
    cout<<"Archivo leido ....\n";
    cout<<"Nodes generados ";
    cout<<"\n\n\n";
}
/*
    Permite leer un archivo ubication se encuentran los valores que se erasean del Btree.
*/
void Btree::eraseKeys(string mensaje){
    //Se abre el archivo
    string ruta = "casosDeBorrado/"+mensaje+".txt";
    char *y = (char *)ruta.c_str();
    ifstream fin(y);
    //En caso de no poder abrir el archivo se le avisa al usuario
    if(fin.fail()){
        cout<<"No se pudo abrir el archivo";
        system("pause");
        exit(1);
    }
    int arcos, a;
    //Se lee la cantidad de numeros que se leeran
    fin>>arcos;
    for(int i = 1; i<=arcos; i++){
	//Se agrega la informacion a la grafica
        fin>>a;
        erase(a);
        cout<<"\n -> Numero borrado: " <<a<<"\n";
    }
    cout<<"\n\n\n";
    cout<<"Archivo leido ....\n";
    cout<<"Valores borrados .....";
    cout<<"\n\n\n";
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Permite draw los Nodes del Btree en pantalla.
*/
void Btree::draw(){
    cout<<"\n\n=========== Btree =============\n\n";
    Queue *Q= new Queue;
    Queue *H =  new Queue;
    Q->nuevo();
    H->nuevo();
	Node *node_next;
	cashDirection *direccion;
	if(raiz != NULL){
        Q->insert(raiz);
        while(!Q->estaVacia()){
            node_next =  Q->take();
            if(node_next->esHoja == true){
                cout<<"Es hoja: ";
            }
            node_next->llaves.draw();
            if(node_next->esHoja == true){
                H->insert(node_next);
            }
            direccion = node_next->direccion.ubicationPrincipio();
            while(direccion  != NULL){
                Q->insert(direccion->direccion);
                direccion = direccion->next;
            }
            cout<<"\n";
        }

        cout<<"Valores: ";
        while(!H->estaVacia()){
            node_next = H->take();
            node_next->llaves.draw();
        }

        cout<<"\n";
	}else{
        cout<<"\nEl Btree esta vacio\n";
	}

	Q->empty();
	H->empty();
    cout<<"\n\n=========== Fin del Btree =============\n";
}

#endif

