#include <iostream>
#include <math.h>

using namespace std;

/*
Formulas
Indice del local(K): ( largo * ancho ) / (alturaTotal * (largo + ancho))
Coeficiente de utilizacion(Cu): 
	Depende de K, factor del techo, paredes checar tabla
FlujoTotal: CantidadLuxes area general ((cocina(200-300) area de trabajo(500)) * area) / (Cu * Cd)
Numero de luminarias(N): FlujoTotal / (NumeroLamparas * Lumenes)
Comprobacion(Em): se escoge N y N - 1 como N1 y N2 ==== (N * NumeroLamparas * Lumenes * Cu * Cd) / area
Si el resultado es mayor que CantidadLuxes
*/
int main(int argc, char *argv[]) {
	// por teclado
	double Cu, largo, ancho, alturaTotal, CantLuxes, Cd, NumeroLamparas, Lumenes;
	// calculado
	double K, FlujoTotal, N, area, N1, N2, numFilas, numColumnas, lar1, anc1;
	int it = 1;
	while(true) {
		cout<<"CALCULO NUMERO: " << it << "\n";
		cout<<"Ingresa largo: "; cin >>largo;
		cout<<"Ingresa ancho: "; cin>>ancho;
		cout<<"Ingresa altura de trabajo: "; cin>>alturaTotal;
		cout<<"Ingresa cantidad luxes requeridos: "; cin>>CantLuxes;
		cout<<"Ingresa coeficiente de conservacion: "; cin>>Cd;
		cout<<"Ingresa numero de lamparas: "; cin>>NumeroLamparas;
		cout<<"Ingresa lumenes de la lampara: "; cin>>Lumenes;
		
		area = largo * ancho;
		K = (largo * ancho) / (alturaTotal *(largo + ancho));
		cout<< "\nIndice de local(K): " << K << "\n";
		cout<<"Ingresar el Cu, segun la tabla: "; cin>> Cu;
		FlujoTotal = (CantLuxes * area) / (Cu * Cd);
		N = ceil(FlujoTotal / (NumeroLamparas * Lumenes));
		
		N1 = (N * NumeroLamparas * Lumenes * Cu * Cd) / area;
		N2 = (N - 1 * NumeroLamparas * Lumenes * Cu * Cd) / area;
				
		// numero de filas
		numFilas = sqrt((N * ancho) / largo);
		numColumnas = (trunc(numFilas * 100) / 100) * largo / ancho;
		
		lar1 = largo / round(numColumnas);
		anc1 = ancho / round(numFilas);
		
		// 1.85 1.1 2.4 100 0.8 1 131 0.245
		cout<< "\nCoeficiente de utilizacion(Cu): " << Cu
			<< "\nFlujo total: " << FlujoTotal
			<< "\nNumero de luminarias(N): " << N
			<< "\nNumero de filas: " << numFilas
			<< "\nNumero de filas redondeado: " << round(numFilas)
			<< "\nNumero de columnas: " << numColumnas
			<< "\nNumero de columnas redondeado: " << round(numColumnas)
			<<"\nLARGO"
			<< "\nDistancia entre luminarias: " << lar1
			<< "\nDistancia de pared a luminaria: " << lar1 / 2
			<<"\nANCHO"
			<< "\nDistancia entre luminarias: " << anc1
			<< "\nDistancia de pared a luminario: " << anc1 / 2
			<< "\nCon respecto a la comprobacion(Em) de resultados: \n";
			if(N1 >= CantLuxes) 
			cout<<"TODO BIEN\n";
		if(N2 < CantLuxes)
			cout<<"Se sugiere usar " << N << " lamparas\n";
		++it;
		system("pause");
		cout<<"\n\n\n\n\n";
	}
	
	return 0;
}

