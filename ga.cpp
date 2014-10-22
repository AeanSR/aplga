#include "aplga.h"

struct lelem_t{
    apl_t* apl;
    float dps;
    int complexity;
} list[130];

float eval(int idx){
    std::string str;
    list[idx].apl->print(str);
    __m128i lh;
    uint32_t sh = strhash(str.c_str(), lh);
    float dps;
    if (checktt(lh, sh)) return .0;
    dps = ocl().run(str);
    if (dps < .0) abort();
    recordtt(lh, sh);
    return dps;
}

void dpssort(int first, int last){
    if (last - first > 1){
        int i = first + 1;
        int j = last;
        float key = list[first].dps;
        while (1){
            while (key > list[j].dps)
                j--;
            while (key < list[i].dps && i<j)
                i++;
            if (i >= j) break;
            auto tmpelem = list[i];
            list[i] = list[j];
            list[j] = tmpelem;
            if (list[i].dps == key)
                j--;
            else
                i++;
        }
        auto tmpelem = list[j];
        list[j] = list[first];
        list[first] = tmpelem;
        if (first < i - 1) dpssort(first, i - 1);
        if (j + 1 < last) dpssort(j + 1, last);
    }
    else{
        if (list[first].dps < list[last].dps){
            auto tmpelem = list[first];
            list[first] = list[last];
            list[last] = tmpelem;
        }
    }
}

void complexitysort(int first, int last){
    if (last - first > 1){
        int i = first + 1;
        int j = last;
        int key = list[first].complexity;
        while (1){
            while (key < list[j].complexity)
                j--;
            while (key > list[i].complexity && i<j)
                i++;
            if (i >= j) break;
            auto tmpelem = list[i];
            list[i] = list[j];
            list[j] = tmpelem;
            if (list[i].complexity == key)
                j--;
            else
                i++;
        }
        auto tmpelem = list[j];
        list[j] = list[first];
        list[first] = tmpelem;
        if (first < i - 1) complexitysort(first, i - 1);
        if (j + 1 < last) complexitysort(j + 1, last);
    }
    else{
        if (list[first].complexity > list[last].complexity){
            auto tmpelem = list[first];
            list[first] = list[last];
            list[last] = tmpelem;
        }
    }
}



int main(){
    int i;
    unsigned int gen = 0;
    
    FILE* fdata = fopen("data.txt", "wb");
    FILE* fbest = fopen("best.txt", "wb");
    
    /* init genetic. */
    for (i = 0; i < 100; i++){
        list[i].apl = new apl_t;
        list[i].dps = eval(i);
        while (list[i].dps <= .0){
            list[i].apl->mutation();
            list[i].dps = eval(i);
        }
    }
    while (1){
        /* chiasma */
        for (i = 100; i < 120; i+=2){
            int j = i + 1;
            int m1 = static_cast<int>(100 * uni_rng());
            int m1r = static_cast<int>(100 * uni_rng());
            m1 = m1 < m1r ? m1 : m1r;
            int m2 = static_cast<int>(100 * uni_rng());
            int m2r = static_cast<int>(100 * uni_rng());
            m2 = m2 < m2r ? m2 : m2r;

            list[i].apl = new apl_t(*list[m1].apl);
            list[j].apl = new apl_t(*list[m2].apl);
            list[i].apl->chiasma(*list[j].apl);
        }
        /* mutation */
        for (i = 120; i < 130; i++){
            int m = static_cast<int>(100 * uni_rng());
            int mr = static_cast<int>(100 * uni_rng());
            m = m < mr ? m : mr;

            list[i].apl = new apl_t(*list[m].apl);
            list[i].apl->mutation();
        }
        /* evaluation */
        for (i = 100; i < 130; i++){
            list[i].dps = eval(i);
            list[i].complexity = list[i].apl->complexity();
        }
        /* sort */
        dpssort(0, 129);
        int j = 0;
        for (i = 1; i < 130; i++){
            if (list[i].dps != list[j].dps){
                if (i - j > 1) complexitysort(j, i - 1);
                j = i;
            }
        }
        if (j < 100) complexitysort(j, 129);
        /* select */
        for (i = 100; i < 130; i++){
            list[i].dps = .0;
            delete list[i].apl;
            list[i].apl = nullptr;
        }
        /* record */
        gen++;
        std::string str;
        list[0].apl->print(str);
        rewind(fbest);
        fprintf(fbest, "DPS %.3f, APL:\n%s\n\n**********\n", list[0].dps, str.c_str());
        printf("Gen %d, Max %.3f, Min %.3f. Best APL:\n%s\n", gen, list[0].dps, list[99].dps, str.c_str());
        fprintf(fdata, "%d, %.3f, %.3f\n", gen, list[0].dps, list[99].dps);
        fflush(fbest);
        fflush(fdata);
    }
}