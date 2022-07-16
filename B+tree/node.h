#ifndef Node_H
#define Node_H

using namespace std;

struct Node;
enum positions {emty,Principio,middle,End};
enum boolean { NO = false, SI = true };

/*
    Esta estructura contiene la informacion de cada elemento de la lista.
*/
struct cashValue{
    int value;
    cashValue *next;
};


/*
    Esta clase forma una lista ordenada con elementos caja.
*/
class listValue{
    private:
        cashValue *principio, *anterior;
        positions ubication;
        boolean encontrado;
        int cuantosVal;

    public:
        listValue();
        ~listValue();

        void _new();
        void empty();

        void find(int a);
        int insert(int a);
        int erase(int a);
        int take(void);
        void draw(void);
        int CuantosVal();
        cashValue* ubicationPrincipio();
};

/*
    Esta estructura contiene la informacion de cada elemento de la lista.
*/
struct cashDirection{
    Node *direccion;
    cashDirection *next;
};

/*
    Esta clase forma una lista ordenada con elementos caja.
*/
class listDirection{
    private:
        cashDirection *principio, *anterior;
        positions ubication;
        boolean encontrado;
        int cuantosDir;

    public:
        listDirection();
        ~listDirection();

        void _new();
        void empty();

        void find(Node *p);
        int insert(Node *p);
        int erase(Node *p);
        void finderase(Node *p);
        Node* take(void);
        int CuantosDir();
        cashDirection* ubicationPrincipio();
};

struct Node{
    listValue llaves;       //Un arreglo que contiene valuees enteros.
    listDirection direccion;  //Un arreglo de direcciones que contiene a sus posibles hijos.
    bool esHoja;        //Indica si el Node es una hoja del arbol.
    Node* padre;        //Indica la direccion del padre del Node.
};


listValue::listValue(){
	principio = NULL;
	anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosVal = 0;
}

listValue::~listValue(){
	cashValue *p, *aux;
	p = principio;
	while(p!= NULL){
        aux = p;
        p = p->next;
        delete aux;
    }
    principio = NULL;
    anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosVal = 0;
}

void listValue::_new(){
    principio = NULL;
	anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosVal = 0;
}

void listValue::empty(){
    cashValue *p, *aux;
	p = principio;
	while(p!= NULL){
        aux = p;
        p = p->next;
        delete aux;
    }
    principio = NULL;
    anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosVal = 0;
}

void listValue::find(int a){
	cashValue *p;
	p = principio;
	if(p == NULL){
		encontrado = NO;
		ubication = emty;
		return;
	}
	while(p){
		if(p->value == a){
			encontrado = SI;
			if(principio == p){
				ubication = Principio;
			}else{
				ubication = middle;
			}
			return;
		}else if(p->value<a){
			anterior = p;
			p = p->next;
		}else{
			encontrado = NO;
			if(principio == p){
				ubication = Principio;
			}else{
				ubication = middle;
			}
			return;
		}
	}
	encontrado = NO;
	ubication = End;
	return;
}

/*
    Esta funcion agrega una caja a la lista ordenada con el value que recibe, en
    la positions que le corresponde.
*/
int listValue::insert(int a){
	cashValue *p;
	find(a);
	if(encontrado == SI){
		return 0;
	}
	p = new cashValue;
	if(ubication == emty){
		principio = p;
		p->next = NULL;
		p->value = a;
	}else if(ubication == Principio){
		p->next = principio;
		principio = p;
		p->value = a;
	}else if(ubication == middle){
		p->next = anterior->next;
		anterior->next = p;
		p->value = a;
	}else{
		p->next = NULL;
		anterior->next = p;
		p->value = a;
	}
	cuantosVal++;
	return 1;
}

/*
    Esta funcion busca el value recibido en la lista para eliminar la caja que
    contenga ese value.
*/
int listValue::erase(int a){
	cashValue *p;
	find(a);
	if(encontrado == NO){
		return 0;
	}
	if(ubication == Principio){
		p = principio;
		principio = p->next;
	}else if(ubication == middle){
		p = anterior->next;
		anterior->next = p->next;
	}else{
		p = anterior->next;
		anterior->next = NULL;
	}
	delete p;
	cuantosVal--;
	return 1;
}
/*
    Esta funcion saca la primera caja de la lista ordenada.
*/
int listValue::take(void){//Saca el ultimo
	cashValue *p, *ultimo = NULL;
	int value;
	if(principio == NULL){
		return 999999999;
	}
	p = principio;
	if(p->next!= NULL){
        while(p->next!= NULL){
            if(p->next->next == NULL){
                ultimo = p;
            }
            p = p->next;
        }
        if(ultimo!= NULL){
            ultimo->next = NULL;
        }
	}else{
        principio = NULL;
	}
	p->next = NULL;
	value = p->value;
	delete p;
	cuantosVal--;
	return value;
}

//Constructor
listDirection::listDirection(){
	principio = NULL;
	anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosDir = 0;
}

//Destructor
listDirection::~listDirection(){
	cashDirection *p, *aux;
	p = principio;
	while(p!= NULL){
        aux = p;
        p = p->next;
        delete aux;
    }
    principio = NULL;
    anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosDir = 0;
}

void listDirection::_new(){
    principio = NULL;
	anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosDir = 0;
}

void listDirection::empty(){
    cashDirection *p, *aux;
	p = principio;
	while(p!= NULL){
        aux = p;
        p = p->next;
        delete aux;
    }
    principio = NULL;
    anterior = NULL;
	ubication = emty;
	encontrado = NO;
	cuantosDir = 0;
}

//Funciones de la clase
/*
    Esta funcion revisa la lista ordenada para determinar la ubicacion de una
    caja en la lista.
*/
void listDirection::find(Node *p){
	cashDirection *q;
	q = principio;
	if(q == NULL){
		encontrado = NO;
		ubication = emty;
		return;
	}
	while(q){
		if(q->direccion == p){
			encontrado = SI;
			if(principio == q){
				ubication = Principio;
			}else{
				ubication = middle;
			}
			return;
		}else if(p->llaves.ubicationPrincipio()!= NULL){
             if(q->direccion->llaves.ubicationPrincipio()->value<p->llaves.ubicationPrincipio()->value){
                anterior = q;
                q = q->next;
            }else{
                encontrado = NO;
                if(principio == q){
                    ubication = Principio;
                }else{
                    ubication = middle;
                }
                return;
            }
		}else{
            anterior = q;
            encontrado = NO;
            if(principio == q){
                ubication = Principio;
            }else if(q->next!= NULL){
                ubication = middle;
            }else{
                ubication = End;
            }
            return;
		}
	}
	encontrado = NO;
	ubication = End;
	return;
}

void listDirection::finderase(Node *p){
	cashDirection *q;
	q = principio;
	if(q == NULL){
		encontrado = NO;
		ubication = emty;
		return;
	}
	while(q){

		if(q->direccion == p){
			encontrado = SI;
			if(principio == q){
				ubication = Principio;
			}else{
				ubication = middle;
			}
			return;
		}else{
            anterior = q;
            q = q->next;
        }
	}
	encontrado = NO;
	ubication = End;
	return;

}

/*
    Esta funcion agrega una caja a la lista ordenada con el value que recibe, en
    la positions que le corresponde.
*/
int listDirection::insert(Node *p){
	cashDirection *q;
	find(p);
	if(encontrado == SI){
		return 0;
	}
	q = new cashDirection;
	if(ubication == emty){
		principio = q;
		q->next = NULL;
		q->direccion = p;
	}else if(ubication == Principio){
		q->next = principio;
		principio = q;
		q->direccion = p;
	}else if(ubication == middle){
		q->next = anterior->next;
		anterior->next = q;
		q->direccion = p;
	}else{
		q->next = NULL;
		anterior->next = q;
		q->direccion = p;
	}
	cuantosDir++;
	return 1;
}

/*
    Esta funcion busca el value recibido en la lista para eliminar la caja que
    contenga ese value.
*/
int listDirection::erase(Node *p){
	cashDirection *q;
	finderase(p);
	if(encontrado == NO){
		return 0;
	}
	if(ubication == Principio){
		q = principio;
		principio = q->next;
	}else if(ubication == middle){
	    q = anterior->next;
	    anterior->next = q->next;
	}else{
		q = anterior->next;
		anterior->next = NULL;
	}
	delete q;
	cuantosDir--;
	return 1;
}

/*
    Esta funcion saca la primera caja de la lista ordenada.
*/
Node* listDirection::take(void){//Saca la ultima direccion
	cashDirection *p, *ultimo = NULL;
	Node * direccion;
	if(principio == NULL){
		return NULL;
	}

	p = principio;
	if(p->next!= NULL){
        while(p->next!= NULL){
            if(p->next->next == NULL){
                ultimo = p;
            }
            p = p->next;
        }
        ultimo->next = NULL;
	}else{
        principio = NULL;
	}
	direccion = p->direccion;
	if(ultimo!= NULL){
        ultimo->next = NULL;
	}
	delete p;
	cuantosDir--;
	return direccion;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Esta funcion esta funcion imprime los valuees de cada caja en la lista.
*/
void listValue::draw(void){
   cashValue *p;
   p = principio;

   if(cuantosVal == 0){
        cout<<"lista vacia.\n\n";
   }else{
       while(p!= NULL){
           cout<<p->value<<" ";
           p = p->next;

       }
   }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
cashValue* listValue::ubicationPrincipio(){
    return principio;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
cashDirection* listDirection::ubicationPrincipio(){
    return principio;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int listDirection::CuantosDir(){
    return cuantosDir;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int listValue::CuantosVal(){
    return cuantosVal;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


