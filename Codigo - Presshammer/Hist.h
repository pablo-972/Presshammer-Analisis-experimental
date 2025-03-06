#include <unordered_map>
#include <vector>
#include <string>

/** Una clase que recoge los valores de latencia en contenedores (bins).
 * Los contenedores son almacenados en una tabla hash.
 * La clave es el número del conteneddor, y el valor es el número de valores de latencia
 * que caen a ese contenedor.
 */

 /** Por ejemplo, observar los valores de latencia almacenados en bins:
  * bins[0] --> valores de latencia [0,10)
  * bins[1] --> valores de latencia [10, 20)
  * ...
  * bins[x] --> valores de latencia [x*10, x*10+10]
  */


  class Hist
  {
    private:
        std::unordered_map<int, int> bins; //Se crea la estrucutra, que es una tabla hash de dos enteros: clave y valor
        int binsize = 10;

    public:
        //Método para inicializar la estructura
        Hist(int binsize); 

        //Metodo para obtener la latencia
        void observe_latency(int latency);

        //Método para obtener un bin completo
        int get_bin(int key);

        //Método para obtener todas las claves de los contenedores. El resultado es un vector de enteros
        std::vector<int> get_all_bin_keys();

        //Método para pasar a csv
        void to_csv(std::string filename);

        //Método para limpiar todos los valores almacenados
        void clear_all_values();
  }