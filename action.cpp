#include "aplga.h"

const char* actionstr[] = {
    "SPELL(bloodthirst)",
    "SPELL(ragingblow)",
    "SPELL(wildstrike)",
    "SPELL(execute)",
};

const int actionnum = sizeof(actionstr) / sizeof(actionstr[0]);

void actionnode_t::cascade_destruction(){
    if (next){
        next->cascade_destruction();
        delete next;
        next = nullptr;
    }
}

int apl_t::length(){
    if (head) return head->length();
    return 0;
}
int actionnode_t::length(){
    if (!next) return 1;
    return next->length() + 1;
}
int apl_t::complexity(){
    if (head) return head->complexity();
    return 0;
}
int actionnode_t::complexity(){
    if (!next) return cond.complexity() + 1;
    return cond.complexity() + 1 + next->complexity();
}
void actionnode_t::addition(){
    if (!next || uni_rng() < 0.25){
        actionnode_t* p = new actionnode_t(static_cast<int>(uni_rng() * actionnum));
        p->next = next;
        next = p;
    }
    else{
        next->addition();
    }
}

void actionnode_t::deletion(){
    if (!next) return;
    if (!next->next || uni_rng() < 0.25){
        auto p = next;
        next = next->next;
        delete p;
    }
    else{
        next->deletion();
    }
}

void actionnode_t::condmutation(){
    if (!next || uni_rng() < 0.25){
        cond.mutation();
    }
    else{
        next->condmutation();
    }
}

void apl_t::mutation(){
    double c = static_cast<double>(length());
    if (uni_rng() < 0.5){
        /* condmutation */
        if (head) head->condmutation();
    }
    if (c > 3.0 + 4.0 * abs(nor_rng())){
        /* delete */
        if (!head) return;
        head->deletion();
    }
    else {
        /* add */
        if (head) head->addition();
        else head = new actionnode_t(static_cast<int>(uni_rng() * actionnum));
    }
};


void actionnode_t::chiasma(actionnode_t** mate){
    double c = uni_rng();
    if (!next || !*mate){
        auto p = next;
        next = *mate;
        *mate = p;
    }
    else if(next && c < 0.125){
        next->chiasma(mate);
    }
    else if (*mate && c < 0.25){
        chiasma(&(*mate)->next);
    }
    else{
        auto p = next;
        next = *mate;
        *mate = p;
    }    
}

void actionnode_t::condchiasma(actionnode_t** mate){
    double c = uni_rng();
    if (!*mate){
        return;
    }
    else if (!next || !(*mate)->next){
        cond.chiasma((*mate)->cond);
    }
    else if (next && c < 0.125){
        next->condchiasma(mate);
    }
    else if (*mate && c < 0.25){
        condchiasma(&(*mate)->next);
    }
    else{
        cond.chiasma((*mate)->cond);
    }
}

void apl_t::chiasma(apl_t& mate){
    if (!head || !mate.head){
        return;
    }
    else if (uni_rng() < 0.5){
        head->chiasma(&mate.head);
    }
    else{
        head->condchiasma(&mate.head);
    }
}

void actionnode_t::print(std::string& str){
    cond.print(str);
    str.append(actionstr[action]);
    str.append(";");
    if (next) next->print(str);
}

void apl_t::print(std::string& str){
    if (head)
        head->print(str);
}

apl_t::apl_t(){
    head = new actionnode_t(static_cast<int>(uni_rng() * actionnum));
    head->addition();
    head->addition();
    head->addition();
}

/*
int main(){
    apl_t apl1, apl2;
    size_t i = 0x1000000;
    while (i--){
        apl1.mutation();
        apl2.mutation();
        apl1.chiasma(apl2);
    }

}
*/