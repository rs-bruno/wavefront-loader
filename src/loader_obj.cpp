#include <cctype>
#include <cstdio>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include "../h/loader_obj.h"
using namespace std;

/*
	Implementación conjunto de caras.
*/
ConjuntoCaras::~ConjuntoCaras(){
	int* il = indicesListas();
	for(int i = 1; i <= il[0]; ++i){
		list<cara>::iterator it = listas[il[i]].begin();
		for(; it != listas[il[i]].end(); ++it){
			delete[] (*it).ind_vertices;
			delete[] (*it).ind_uvs;
			delete[] (*it).ind_normals;
		}
	}
	listas.clear();
}
void ConjuntoCaras::instestarCara(cara c){
	listas[c.vertex_count].push_back(c);
}
int* ConjuntoCaras::indicesListas(){
	int* ret = new int[listas.size()+1];
	ret[0] = listas.size();
	map<int , list<cara>>::iterator it = listas.begin();
	for(int i = 1; i <= listas.size(); ++i){
		ret[i] = it->first;
		it++;
	}
	return ret;
}
list<cara>& ConjuntoCaras::devolverLista(int cant_vertices){
	return listas[cant_vertices];
}

/*
	Implementación funciones del módulo.
*/
modelo load_modelo(char* path){
	modelo ret;
	ret.vertex_count = 0;
	ret.uv_count = 0;
	ret.normal_count = 0;

	FILE* model_file;
	char buff[512];
	list<float> l_vertices;
	list<float> l_uvs;
	list<float> l_normals;

	regex re1("v ([-]?[0-9]*\\.[0-9]*) ([-]?[0-9]*\\.[0-9]*) ([-]?[0-9]*\\.[0-9]*)");
	regex re2("vt ([-]?[0-9]*\\.[0-9]*) ([-]?[0-9]*\\.[0-9]*)");
	regex re3("vn ([-]?[0-9]*\\.[0-9]*) ([-]?[0-9]*\\.[0-9]*) ([-]?[0-9]*\\.[0-9]*)");
	regex re4("f (([0-9]*)/([0-9]*)/([0-9]*)\\s)*");
	cmatch m;

	//Se recorre la lista de lineas en busca de las que matchean la regex1.
	model_file  = fopen(path, "r");
	while(fgets(buff, sizeof(buff), model_file)){
		if(regex_search(buff, m, re1)){
			l_vertices.push_back(stof(m[1].str()));
			l_vertices.push_back(stof(m[2].str()));
			l_vertices.push_back(stof(m[3].str()));
		}
	}

	//Se recorre la lista de lineas en busca de las que matchean la regex2.
	fseek(model_file, 0, SEEK_SET);
	while(fgets(buff, sizeof(buff), model_file)){
		if(regex_search(buff, m, re2)){
			l_uvs.push_back(stof(m[1].str()));
			l_uvs.push_back(stof(m[2].str()));
		}
	}

	//Se recorre la lista de lineas en busca de las que matchean la regex3.
	fseek(model_file, 0, SEEK_SET);
	while(fgets(buff, sizeof(buff), model_file)){
		if(regex_search(buff, m, re3)){
			l_normals.push_back(stof(m[1].str()));
			l_normals.push_back(stof(m[2].str()));
			l_normals.push_back(stof(m[3].str()));
		}
	}
	
	//Se pasa la lista de vertices a formato de arreglo
	ret.vertex_count = l_vertices.size() / 3;
	ret.vertex_arr = new float[l_vertices.size()];
	list<float>::iterator it = l_vertices.begin();
	for(int i = 0; i < l_vertices.size(); ++i)
		ret.vertex_arr[i] = *(it++);

	//Se pasa la lista de uvs a formato de arreglo
	ret.uv_count = l_uvs.size() / 2;
	ret.uv_arr = new float[l_uvs.size()];
	it = l_uvs.begin();
	for(int i = 0; i < l_uvs.size(); ++i)
		ret.uv_arr[i] = *(it++);

	//Se pasa la lista de normales a formato de arreglo
	ret.normal_count = l_normals.size() / 3;
	ret.normal_arr = new float[l_normals.size()];
	it = l_normals.begin();
	for(int i = 0; i < l_normals.size(); ++i)
		ret.normal_arr[i] = *(it++);
	
	//Se recorre la lista de lineas en busca de las que matchean la regex4 y se agrega la cara al conjunto de caras.
	fseek(model_file, 0, SEEK_SET);
	while(fgets(buff, sizeof(buff), model_file)){
		if(regex_search(buff, m, re4)){
			//Parseo linea manualmente (ya se que tiene la forma de la regex4).
			list<int> ind_ve;
			list<int> ind_uv;
			list<int> ind_nm;
			char* aux = &buff[2] ; // saltea la f y el espacio
			int buff = -1;
			int turno = 0;
			while(*aux != '\0'){
				if(*aux == '/'|| *aux == ' ' || *aux == '\n'){
					/*
						Se resta uno a buff porque los indices de .obj arrancan en 1 mientras que los de
						la clase Modelo arrancan en 0.
					*/
					switch(turno){
						case 0:	ind_ve.push_back(buff - 1);
							break;
						case 1: ind_uv.push_back(buff - 1);
							break;
						case 2: ind_nm.push_back(buff - 1);
							break;
					}
					turno = (++turno) % 3;
					buff = -1;
				}
				else if(isdigit(*aux)){
					if(buff == -1 ) buff = 0;
					buff = buff*10 + (*aux - '0');
				}
				++aux;
			}

			//Agrego cara al modelo resultado.
			cara c;
			c.vertex_count = ind_ve.size();

			c.ind_vertices = new int[ind_ve.size()];
			int i = 0;
			for(list<int>::iterator it = ind_ve.begin(); it != ind_ve.end(); ++it){
				c.ind_vertices[i++] = *it;
			}

			c.ind_uvs = new int[ind_ve.size()];
			i = 0;
			for(list<int>::iterator it = ind_uv.begin(); it != ind_uv.end(); ++it){
				c.ind_uvs[i++] = *it;
			}

			c.ind_normals = new int[ind_ve.size()];
			i = 0;
			for(list<int>::iterator it = ind_nm.begin(); it != ind_nm.end(); ++it){
				c.ind_normals[i++] = *it;
 			}
			
			ret.caras.instestarCara(c);
		}
	}
	fclose(model_file);
	return  ret;
}

void print_cara(cara c){
	cout << "f ";
	for(int i = 0; i < c.vertex_count; ++i){
		cout << c.ind_vertices[i] << "/" << c.ind_uvs[i] << "/" << c.ind_normals[i] << " ";
	}
	cout << endl;
}

void print_modelo(modelo m){
	cout << "vertex_arr:" << "(" << m.vertex_count << ")" << endl;
	for(int i = 0; i < m.vertex_count; ++i){
		cout << i << ": (" << m.vertex_arr[3*i] << ", " << m.vertex_arr[3*i+1] << ", " << m.vertex_arr[3*i+2] << ") " << endl;
	}
	cout << endl;

	cout << "uv_arr:" << "(" << m.uv_count << ")" << endl;
	for(int i = 0; i < m.uv_count; ++i){
		cout << i << ": (" << m.uv_arr[2*i] << ", " << m.uv_arr[2*i+1] << ") " << endl;
	}
	cout << endl;

	cout << "normal_arr:" << "(" << m.normal_count << ")" <<endl;
	for(int i = 0; i < m.normal_count; ++i){
		cout << i << ": (" << m.normal_arr[3*i] << ", " << m.normal_arr[3*i+1] << ", " << m.normal_arr[3*i+2] << ") " << endl;
	}
	cout << endl;

	int* indices_listas = m.caras.indicesListas();
	for(int i = 1; i <= indices_listas[0]; ++i){
		cout << "Lista de poligonos con " << indices_listas[i] << " vertices: " << endl;
		list<cara> l = m.caras.devolverLista(indices_listas[i]);
		for(list<cara>::iterator it = l.begin(); it != l.end(); ++it){
			print_cara(*it);
		}
		cout << endl;
	}
}

void free_modelo(modelo m){
	delete[] m.vertex_arr;
	delete[] m.uv_arr;
	delete[] m.normal_arr;
}