#include <iostream>
#include <list>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <string>

#define MAX_ITER 20

class Graph {
    
    private:
        int V;
        std::list<int> *adj;

    public:
        Graph(int V) { 
            this->V = V; 
            adj = new std::list<int>[V]; 
        }
        ~Graph() {
            delete[] adj;
        }

        template<typename T>
        T findMax(T *arr, size_t n) {
            return *std::max_element(arr, arr+n);            
        }

        void addEdge(int v, int w) {
            adj[v].push_back(w);
            adj[w].push_back(v);
        }

        void color() {
            int colored[V];
            bool toColor;
            int minColors = V;

            omp_set_num_threads(12);

            auto end = std::chrono::steady_clock::now();
            double ompstart = omp_get_wtime();
            double ompstop;
            auto start = std::chrono::steady_clock::now();
            
            //Pętla powtarzająca obliczenia dodana w 4 wersji kodu
            for (int n=0; n<MAX_ITER; ++n){    
                colored[0] = 0;
                for (int i=1; i<V; ++i) {
                    colored[i] = -1;
                }
                toColor = true;       
                while (toColor){

                    //Próba zrównoleglenia algorytmu sekwencyjnego (1 wersja kodu)
                    #pragma omp parallel
                    {   
                        bool neighColor[V];
                        //Dyrektywy schedule dodane w 5 wersji kodu
                        //Dodatkowa równoległa pętla dodana w 3 wersji kodu
                        #pragma omp for schedule(static)
                        for (int i=0; i<V; ++i){
                            neighColor[i] = false;
                        }
                        //W 1 wersji kodu równoległa jedynie ta pętla
                        #pragma omp for schedule(static)
                        for (int i=1; i<V; ++i) {
                            if (colored[i] != -1){
                                continue;
                            }
                            std::list<int>::iterator j;

                            for (j=adj[i].begin(); j != adj[i].end(); ++j) {
                                if (colored[*j] != -1) {
                                    neighColor[colored[*j]] = true;
                                }
                            }

                            int k;
                            for (k=0; k<V; ++k) {
                                if (!neighColor[k]) {
                                    break;
                                }
                            }

                            colored[i] = k;
                        
                            for (j=adj[i].begin(); j != adj[i].end(); ++j) {
                                if (colored[*j] != -1) {
                                    neighColor[colored[*j]] = false;
                                }
                            }
                        }
                    }
                    
                    toColor = false;

                    //Wykrywanie błędnego pokolorowania spowodowanego równoległością dodane w 2 wersji kodu  
                    #pragma omp parallel for schedule(static) //reduction(+:toColor)
                    for (int i=0; i<V; ++i) {
                        std::list<int>::iterator j;

                        for (j=adj[i].begin(); j != adj[i].end(); ++j) {
                            if (*j != i && colored[*j] == colored[i]) {
                                //std::cout << "Conflict: " << i << " & " << *j << std::endl;
                                if (i > *j) {
                                    colored[*j] = -1;
                                } 
                                else {
                                    colored[i] = -1;
                                }
                                toColor = true;
                            }
                        }
                    }

                }

                /*for (int i=0; i<V; ++i) {
                    std::cout << "Vertex " << i << " is color " << colored[i] << std::endl;
                }*/
                end = std::chrono::steady_clock::now();
                ompstop = omp_get_wtime();

                std::cout << "Elapsed time after full iteration in microseconds: "
                    << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                    << " us" << std::endl;

                std::cout << "Processing time: " << (ompstop - ompstart) * 10e6 << std::endl;

                std::cout << "Colors used: " << findMax(colored, V)+1 << std::endl;
                minColors = std::min(minColors, findMax(colored, V)+1);
            }
            std::cout << "Min colors used: " << minColors << std::endl;
        }   
};

int main(int argc, char** argv){

    if (argc < 3){
        std::cout << "To few arguments. Use: ./par [V] [file]" << std::endl;
        exit(1);
    }

    std::ifstream file;
    std::string line;
    size_t pos;
    int vert1, vert2;
    std::string sVert;

    Graph g(std::stoi(argv[2]));

    /*g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 2);
    g.addEdge(1, 4);
    g.addEdge(2, 4);
    g.addEdge(4, 3);*/

    file.open(argv[1]);

    int i = 1;

    while (file) {
        
        getline(file, line);
        std::stringstream ss(line);
        ss >> sVert;
        if (sVert == "e") {
            ss >> vert1;
            ss >> vert2;
            g.addEdge(vert1-1, vert2-1);
            //std::cout << i << " e " << vert1-1 << " " << vert2-1 << std::endl;
            ++i;
        }
        
        
    }

    file.close();

    

    g.color();

    return 0;
}