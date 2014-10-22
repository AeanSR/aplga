#include "aplga.h"

enum{
    COND_AND,
    COND_OR,
    COND_NOT,
    COND_LEAF
};

void condnode_t::cascade_destruction(){
    if (l){
        l->cascade_destruction();
        delete l;
        l = nullptr;
    }if (r){
        r->cascade_destruction();
        delete r;
        r = nullptr;
    }
};
const char* condstr[] = {
    "power_check(rti,110)",
    "power_check(rti,90)",
    "power_check(rti,60)",
    "power_check(rti,50)",
    "power_check(rti,40)",
    "power_check(rti,30)",
    "power_check(rti,20)",
    "REMAIN(gcd)>=FROM_MILLISECONDS(200)",
    "REMAIN(gcd)>=FROM_MILLISECONDS(400)",
    "REMAIN(gcd)>=FROM_MILLISECONDS(800)",
    "REMAIN(gcd)>=FROM_MILLISECONDS(1000)",
    "REMAIN(gcd)>=FROM_MILLISECONDS(1200)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(4)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(3.5)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(3)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(2.5)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(2)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(1.5)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(1)",
    "REMAIN(bloodthirst.cd)>=FROM_SECONDS(0.5)",
    "REMAIN(bloodthirst.cd)==FROM_SECONDS(0)",
    "rti->player.ragingblow.stack==2",
    "rti->player.ragingblow.stack>=1",
    "rti->player.ragingblow.stack<=1",
    "rti->player.ragingblow.stack==0",
    "UP(enrage.expire)",
    "REMAIN(enrage.expire)<=FROM_SECONDS(1.5)",
    "REMAIN(enrage.expire)<=FROM_SECONDS(3)",
    "REMAIN(enrage.expire)<=FROM_SECONDS(4.5)",
    "REMAIN(enrage.expire)<=FROM_SECONDS(6)",
    "REMAIN(enrage.expire)<=FROM_SECONDS(7.5)",
    "rti->player.bloodsurge.stack==2",
    "rti->player.bloodsurge.stack>=1",
    "rti->player.bloodsurge.stack<=1",
    "rti->player.bloodsurge.stack==0",
    "UP(sudden_death.expire)",
    "enemy_health_percent(rti)<=5.0f",
    "enemy_health_percent(rti)<20.0f",
    "enemy_health_percent(rti)>=20.0f",
};
const int condnum = sizeof(condstr) / sizeof(condstr[0]);

int condnode_t::complexity(){
    int c = 1;
    if (l){
        c += l->complexity();
    }
    if (r){
        c += r->complexity();
    }
    return c;
}

int cond_t::complexity(){
    if (root) return root->complexity();
    return 0;
}

void cond_t::print(std::string& str){
    if (root){
        str.append("if(");
        root->print(str);
        str.append(")");
    }
}

void condnode_t::print(std::string& str){
    switch (type){
    case COND_AND:
        str.append("(");
        l->print(str);
        str.append("&&");
        r->print(str);
        str.append(")");
        break;
    case COND_OR:
        str.append("(");
        l->print(str);
        str.append("||");
        r->print(str);
        str.append(")");
        break;
    case COND_NOT:
        str.append("!");
        l->print(str);
        break;
    case COND_LEAF:
        str.append(condstr[leaf]);
    }
}

void condnode_t::alternate(){
    if (type == COND_LEAF){
        leaf = static_cast<int>(uni_rng() * condnum);
        return;
    }
    if (type == COND_NOT){
        l->alternate();
        return;
    }
    double c = uni_rng();
    if (c < 0.2){
        type = (type == COND_AND ? COND_OR : COND_AND);
    }
    else if (c < 0.6){
        l->alternate();
    }
    else {
        r->alternate();
    }
}

void condnode_t::addition(){
    if (type == COND_LEAF){
        if (uni_rng() < 0.2){
            l = new condnode_t(COND_LEAF, leaf);
            type = COND_NOT;
        }
        else{
            l = new condnode_t(COND_LEAF, leaf);
            r = new condnode_t(COND_LEAF, static_cast<int>(uni_rng() * condnum));
            type = uni_rng() < 0.5 ? COND_AND : COND_OR;
            leaf = 0;
        }
    }
    else if (type == COND_NOT){
        if (uni_rng() < 0.5){
            condnode_t* newl = new condnode_t(COND_NOT, 0);
            newl->l = l;
            newl->r = nullptr;
            l = newl;
            r = new condnode_t(COND_LEAF, static_cast<int>(uni_rng() * condnum));
            type = uni_rng() < 0.5 ? COND_AND : COND_OR;
        }
        else{
            l->addition();
        }
    }
    else{
        if (uni_rng() < 0.2){
            condnode_t* newl = new condnode_t(type, 0);
            newl->l = l;
            newl->r = r;
            l = newl;
            r = new condnode_t(COND_LEAF, static_cast<int>(uni_rng() * condnum));
            type = uni_rng() < 0.5 ? COND_AND : COND_OR;
        }
        else{
            if (uni_rng() < 0.5) l->addition();
            else r->addition();
        }
    }
}

void condnode_t::deletion(){
    if (type == COND_NOT){
        if (l->type == COND_LEAF || uni_rng() < 0.2){
            condnode_t* p = l;
            type = p->type;
            leaf = p->leaf;
            l = p->l;
            r = p->r;
            delete p;
        }
        else{
            l->deletion();
        }
    }
    else{
        if (type == COND_LEAF) return;
        if ((l->type == COND_LEAF && r->type == COND_LEAF) || uni_rng() < 0.2){
            if (uni_rng() < 0.5){
                condnode_t* p = l;
                type = p->type;
                leaf = p->leaf;
                l = p->l;
                r->cascade_destruction();
                delete r;
                r = p->r;
                delete p;
            }
            else{
                condnode_t* p = r;
                type = p->type;
                leaf = p->leaf;
                r = p->r;
                l->cascade_destruction();
                delete l;
                l = p->l;
                delete p;
            }
        }
        else if (r->type == COND_LEAF || uni_rng() < 0.5){
            l->deletion();
        }
        else{
            r->deletion();
        }
    }
}

void cond_t::mutation(){
    double c = static_cast<double>(complexity());
    if (uni_rng() < 0.2){
        /* alternate */
        if (root) root->alternate();
    }else if (c > 3.0 * abs(nor_rng())){
        /* delete */
        if (!root) return;
        if (c <= 1.0){
            delete root;
            root = nullptr;
        }
        else{
            root->deletion();
        }
    }
    else {
        /* add */
        if (root) root->addition();
        else root = new condnode_t(COND_LEAF, static_cast<int>(uni_rng() * condnum));
    }
};

void node_chiasma(condnode_t** a, condnode_t** b){
    if ((!*a || !*b)) return;
    double c = uni_rng();
    if (c < 0.2){
        auto p = *a;
        *a = *b;
        *b = p;
    }
    else if (*a && (*a)->l && c < 0.4){
        node_chiasma(&((*a)->l), b);
    }
    else if (*a && (*a)->r && c < 0.6){
        node_chiasma(&((*a)->r), b);
    }
    else if (*b && (*b)->l && c < 0.8){
        node_chiasma(a, &((*b)->l));
    }
    else if (*b && (*b)->r){
        node_chiasma(a, &((*b)->r));
    }
    else{
        auto p = *a;
        *a = *b;
        *b = p;
    }
}

void cond_t::chiasma(cond_t& mate){
    node_chiasma(&root, &mate.root);
}

/*
int main(){
    cond_t cond1, cond2;
    size_t i = 0x1000000;
    while (i--){
        cond1.mutation();
        cond2.mutation();
        cond1.chiasma(cond2);
    }
}*/