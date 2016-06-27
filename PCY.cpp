#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <bitset>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <tuple>

#include <cstdlib>
#include <cmath>

#define FNV1_offset32 2186136261u; //default times gia 32bit ekdosi
#define FNV1_prime32 18777619u;
#define MASK(BITS) (((unsigned int)1<<BITS)-1)
#define BITS 22 //bits για αναπαράσταση κουβάδων, < 32

//κατακερματισμός tuples γιατί δεν έχει έτοιμο η C++
namespace std {
    namespace {
        template <class T>
        inline void hash_combine(std::size_t &seed, T const &v) {
            //ένωση κατακερματισμένων στοιχείων ενός tuple σύμφωνα με boost για αποφυγή συγκρούσεων
            seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }

        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl {
            static void apply(size_t &seed, Tuple const &tuple) {
                HashValueImpl<Tuple, Index-1>::apply(seed, tuple);
                hash_combine(seed, std::get<Index>(tuple));
            }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple,0> {
            static void apply(size_t &seed, Tuple const &tuple) {
                hash_combine(seed, std::get<0>(tuple));
            }
        };
    }

    template <typename ... TT>
    struct hash<std::tuple<TT...>> {
        size_t
        operator()(std::tuple<TT...> const &tt) const {
            size_t seed = 0;
            HashValueImpl<std::tuple<TT...> >::apply(seed, tt);
            return seed;
        }
    };
}

using namespace std;

//Fowler–Noll–Vo συνάρτηση κατακερματισμού (έκδοση για 32bits)
unsigned int FNV1a_hash32(unsigned int number) {
    unsigned char* bytes = (unsigned char*)&number;

    unsigned int hash = FNV1_offset32;
    for(int i = 0; i < sizeof(int); ++i) {
        hash ^= bytes[i];
        hash *= FNV1_prime32;
    }

    return hash;
}

//παίρνουμε την 32-bit έξοδο της hash συνάρτησης και κάνουμε xor-fold σε όσα bits χρειαζόμαστε,
//το οποίο σύμφωνα με τους δημιουργούς δίνει καλύτερη κατανομή στοιχείων από hash mod 2^BITS
//Για Ν κουβάδες το hash Ν/2 διαφορετικών στοιχείων δίνει 2% συγκρούσεις
unsigned int FNV1a_hash(int number, short bits) {
    unsigned int hash = FNV1a_hash32(number); 
    hash = (hash>>bits) ^ (hash  &MASK(bits));

    return hash;
}

//Ακολουθεί την λογική της getline, διαβάζει μια γραμμή και επιστρέφει ένα διάνυσμα με τα προϊόντα ταξινομημένα
bool getbasket(ifstream &input, vector<int> &vec) {
    vec.clear(); //καθαρισμα καλαθιού για την εισαγωγή προϊόντων της νέας γραμμής

    string line;
    if(!getline(input, line)) return false;
    
    char buff[10]; short i = 0;
    for(char c : line) {
        if(c == ' ') {
            buff[i] = 0;
            vec.push_back(atoi(buff));
            i = 0;
        }
        else {
            buff[i++] = c;
        }
    }

    //Για απαλοιφή διπλότυπων
    sort(vec.begin(), vec.end());
    vec.erase(unique(vec.begin(), vec.end()), vec.end());
    
    return true;
}

template<size_t n_buckets>
void PCY_phase1(ifstream &input, unordered_map<int,int> &count_singles, bitset<n_buckets> &bit_vector, int min_support) {
    int* bucket = new int[n_buckets]();

    vector<int> basket;
    while(getbasket(input, basket)) {
        for(int i = 0, len = basket.size(); i < len; ++i) {
            ++count_singles[basket[i]];
            for(int j = i+1; j < len; ++j) {
                //κάνω ένα γρήγορο hash των A+B id των προϊόντων χωρίς να ακολουθήσω κάποια πολιτική hash combine,
                //την οποία χρησιμοποιώ για τα unordered_map των ζευγαριών και τριπλετών
                int hash = FNV1a_hash(basket[i] + basket[j], BITS);
                //Η δημιουργία του bit_vector προσπελαύνοντας τα hash buckets είναι ακριβή (2^22 buckets)
                //οπότε ελέγχω την συνθήκη ελάχιστης στήριξης καθώς αυξάνονται οι τιμές των buckets 
                if(++bucket[hash] >= min_support) bit_vector.set(hash, 1);
            }
        }
    }

    delete[] bucket;

    //Κλάδεμα μη συχνών αντικειμένων
    for(auto it = count_singles.cbegin(); it != count_singles.cend();) {
        if(it->second < min_support)
            count_singles.erase(it++);
        else
            ++it;
    }
}

template<size_t n_buckets>
void PCY_phase2(ifstream &input, unordered_map<int,int> &count_singles, unordered_map<tuple<int,int>, int> &count_pairs, bitset<n_buckets> &bit_vector, int min_support) {
    vector<int> basket;
    while(getbasket(input, basket)) {
        for(int i = 0, len = basket.size(); i < len; ++i) {
            for(int j = i+1; j < len; ++j) {
                //Η count είναι γρήγορη μέθοδος για να βρεθεί αν υπάρχει το κλειδί
                if(count_singles.count(basket[i]) && count_singles.count(basket[j]) && bit_vector[FNV1a_hash(basket[i] + basket[j], BITS)]) {

                    //Το καλάθι είναι ταξινομημένο οπότε δεν έχω προβλήματα τύπου {22,34} !== {34,22}
                    ++count_pairs[make_tuple(basket[i], basket[j])];
                }
            }
        }
    }

    //Γράψιμο σε αρχείο εξόδου τα συχνά, κλάδεμα τα άλλα
    ofstream pair_output("qualified_pairs.txt");
    pair_output << "Item1\tItem2\tFrequency\r\n";
    for(auto it = count_pairs.cbegin(); it != count_pairs.cend();) {
        if(it->second >= min_support) {
            pair_output << get<0>(it->first) << "\t";
            pair_output << get<1>(it->first) << "\t";
            pair_output << it->second << "\r\n";
            ++it;
        }
        else {
            count_pairs.erase(it++);
        }
    }
    pair_output.close();
}

void PCY_phase3(ifstream &input, unordered_map<tuple<int,int>, int> &count_pairs) {
    unordered_map<tuple<int,int,int>, int> count_triplets;
    
    vector<int> basket;
    while(getbasket(input, basket)) {
        for(int i = 0, len = basket.size(); i < len; ++i) {
            for(int j = i+1; j < len; ++j) {
                for(int k = i+2; k < len; ++k) {
                    if(/**/count_pairs.count(make_tuple(basket[i], basket[j])) 
                        && count_pairs.count(make_tuple(basket[i], basket[k]))
                        && count_pairs.count(make_tuple(basket[j], basket[k]))) {
                        ++count_triplets[make_tuple(basket[i], basket[j], basket[k])];
                    }
                }
            }
        }
    }

    ofstream triplet_output("qualified_triplets.txt");
    triplet_output << "Item1\tItem2\tItem3\tFrequency\r\n";
    for(auto &it : count_triplets) {
        triplet_output << get<0>(it.first) << "\t";
        triplet_output << get<1>(it.first) << "\t";
        triplet_output << get<2>(it.first) << "\t";
        triplet_output << it.second << "\r\n";
    }
    triplet_output.close();
}

void PCY(ifstream &input, int min_support) {
    const size_t n_buckets = pow(2, BITS);

    unordered_map<tuple<int,int>, int> count_pairs;
    {
        unordered_map<int,int> count_singles;
        bitset<n_buckets> bit_vector;

        PCY_phase1<n_buckets>(input, count_singles, bit_vector, min_support);
        input.clear(); input.seekg(0, input.beg); //επαναφορά δρομέα στην αρχή του αρχείου
        PCY_phase2<n_buckets>(input, count_singles, count_pairs, bit_vector, min_support);
        input.clear(); input.seekg(0, input.beg);
    } //αφαίρεση count_singles και bit_vector γιατί δεν χρειάζονται στην φάση για 3 στοιχεία και απελευθερώνεται μεγάλο κομμάτι μνήμης
    PCY_phase3(input, count_pairs);
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        cout << "Prepei na dothei h elaxisti stiriksi, to arxeio pou diavazetai einai to 'baskets.txt'" << endl;
        exit(0);
    }

    int min_support = atoi(argv[1]);
    if(min_support < 0) {
        cout << "Prepei na dothei sosti h elaxisti stiriksi >= 0" << endl;
        exit(0);
    }
    
    ifstream input("baskets.txt");
    PCY(input, min_support);
    input.close();

    return 0;
}