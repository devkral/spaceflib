#include "misc.hpp"
#include "thirdparty/Sux/rank9sel.h"

#include <cmath>

uint64_t RSBitmap::calc_blocks(uint64_t nbits) noexcept {
    if (nbits==0)
        return 0;
    // if nbits/pos align don't use +1 block
    if (nbits%64!=0)
        return (nbits/64)+1;
    else
        return (nbits/64);
}

RSBitmap::RSBitmap(const uint64_t nbits) : nbits_(nbits),
    blocks_(calc_blocks(nbits)),
    size_slate(std::max<uint64_t>(log(blocks_), 1)),
    amount_slates(blocks_/size_slate+(blocks_%size_slate!=0 ? 1 : 0)){
    if(n()==0)
        return;
    this->array = new uint64_t[blocks()];
    this->cdarray = new ConstTimeArray<uint32_t>(amount_slates, 0);
    assert(find_slate(nbits)==cdarray->n());
    // cdarray should be active
    assert(this->cdarray!=nullptr && this->rsarray==nullptr);
}

RSBitmap::RSBitmap(const RSBitmap& other): nbits_(other.nbits_),
blocks_(other.blocks_),
size_slate(other.size_slate),
amount_slates(other.amount_slates),
amount_1(other.amount_1)
{
    if(n()==0)
        return;
    this->array = new uint64_t[blocks()];
    std::copy(other.array, other.array+blocks(), this->array);
    if (other.cdarray){
        this->cdarray = new ConstTimeArray<uint32_t>(other.cdarray->copy());
    }
    if (other.rsarray){
        this->rsarray = new rank9sel(array, blocks()*64);
    }
    // exactly one of these structures may be valid
    assert((this->cdarray==nullptr) != (this->rsarray==nullptr));
};

RSBitmap& RSBitmap::operator=(RSBitmap&& other) noexcept{
    if (this != &other){
        if(this->array)
            delete[] this->array;
        if(this->cdarray){
            delete this->cdarray;
            this->cdarray = nullptr;
        }
        if(this->rsarray){
            delete this->rsarray;
            this->rsarray = nullptr;
        }
        nbits_ = other.nbits_;
        blocks_ = other.blocks_;
        size_slate = other.size_slate;
        amount_slates = other.amount_slates;
        amount_1 = other.amount_1;
        this->array=other.array;
        other.array=nullptr;
        if (other.cdarray){
            this->cdarray = other.cdarray;
            other.cdarray = nullptr;
        }
        if (other.rsarray){
            this->rsarray = other.rsarray;
            other.rsarray = nullptr;
        }
        // exactly one of these structures may be valid
        assert((this->cdarray==nullptr) != (this->rsarray==nullptr));
    }
    return *this;
}


RSBitmap::RSBitmap(RSBitmap&& other): nbits_(other.nbits_),
blocks_(other.blocks_),
size_slate(other.size_slate),
amount_slates(other.amount_slates),
amount_1(other.amount_1){
    this->array=other.array;
    other.array=nullptr;
    if(n()==0)
        return;

    if (other.cdarray){
        this->cdarray = other.cdarray;
        other.cdarray = nullptr;
    }
    if (other.rsarray){
        this->rsarray = other.rsarray;
        other.rsarray = nullptr;
    }
    // exactly one of these structures may be valid
    assert((this->cdarray==nullptr) != (this->rsarray==nullptr));
}

RSBitmap::~RSBitmap(){
    if(array)
        delete[] array;
    if(cdarray)
        delete cdarray;
    if(rsarray)
        delete rsarray;

}

bool RSBitmap::get(const uint64_t pos) const{
    // return always false if bigger than nbits, uninitialized or invalid
    if(pos>n() || pos==0 || !is_pos_init(pos))
        return false;
    uint64_t posarray = pos - 1;
    // 1 is uint64_t
    return (array[posarray/64]&(1ull << (posarray%64)))>0;
}
// intern
void RSBitmap::_init_slate(const uint64_t slate){
    assert(slate>0);
    if (cdarray->is_init(slate))
        return;
    uint64_t base = (slate-1)*size_slate;
    // slate can have not full size
    uint64_t limiter = std::min(base+size_slate, blocks());
    for(uint64_t counter=base; counter<limiter; counter++){
        array[counter] = 0;
    }
    // mark initialized
    cdarray->set(slate, 0);
}

void RSBitmap::set(const uint64_t pos, const bool state){
    if(pos>n() || pos==0)
        return;
    // nullarray protection
    SFLCHECK(cdarray)
    uint64_t slate = find_slate(pos);
    // initialize if required
    this->_init_slate(slate);
    uint64_t posarray = pos-1;
    bool old_state = get(pos);
    // 1 is uint64_t
    uint64_t bitm = (1ull << (posarray%64));
    if (state)
        array[posarray/64] |= bitm;
    else
        array[posarray/64] &= ~bitm;

    if (old_state && !state){
        --amount_1;
        --cdarray->ref(slate);
    } else if (!old_state && state){
        ++amount_1;
        ++cdarray->ref(slate);
    }
}
void RSBitmap::flip(const uint64_t pos){
    if(pos>n() || pos==0)
        return;
    // nullarray protection
    SFLCHECK(cdarray)
    uint64_t slate = find_slate(pos);
    // initialize if required
    this->_init_slate(slate);
    uint64_t posarray = pos-1;
    // 1 is uint64_t, shifted
    uint64_t bitm = 1ull << (posarray%64);
    array[posarray/64] ^= bitm;
    bool new_state = get(pos);
    // with flip old state is known
    if (!new_state){
        --amount_1;
        --cdarray->ref(slate);
    } else if (new_state){
        ++amount_1;
        ++cdarray->ref(slate);
    }
}

uint64_t RSBitmap::get_n(const uint64_t pos, const uint8_t nblock) const{
    SFLCHECK(n()%nblock==0)
    SFLCHECK(nblock<=64)
    SFLCHECK(nblock>0)
    if (pos==0 || n()==0)
        return 0;
    uint64_t posarray = pos-1;
    uint64_t start = posarray*nblock;
    uint64_t end = start+nblock-1;
    if (end>=n())
        return 0;
    uint8_t start_rest = (start%64);
    uint8_t next_rest = (64-start_rest);

    uint64_t ret=0;
    // create and move search block
    uint64_t bitm1=((-1ull)>>(64-nblock))<<(start_rest);
    if (is_pos_init(start+1))
        ret |= (array[start/64]&bitm1)>>start_rest;
    // search block is incomplete
    if (find_slate(start+1) != find_slate(end+1) && is_pos_init(end+1)){
        // create new search block
        // TODO: -1 where does it come from?
        uint64_t bitm_next=((-1ull)>>(start_rest-1));
        ret |= (((array[end/64]&bitm_next))<<next_rest);
    }
    return ret;
}

void RSBitmap::set_n(const uint64_t pos, const uint8_t nblock, const uint64_t value){
    SFLCHECK(cdarray)
    SFLCHECK(n()%nblock==0)
    SFLCHECK(nblock<=64)
    SFLCHECK(nblock>0)
    // check overflows with nblock=64
    SFLCHECK(nblock==64 || value<(1<<nblock))
    if (pos==0 || n()==0)
        return;
    uint64_t posarray = pos-1;
    uint64_t start = posarray*nblock;
    uint64_t end = start+nblock-1;
    if (end>=n())
        return;
    uint8_t start_rest = (start%64);
    uint8_t next_rest = (64-start);
    uint64_t slate = find_slate(start+1);
    uint64_t old_value = get_n(pos, nblock);
    int16_t diff;

    // create and move search block and invert
    uint64_t bitm1=~(((-1ull)>>(64-nblock))<<(start_rest));
    this->_init_slate(slate);

    array[start/64] = (value<<start_rest) | (array[start/64]&bitm1);
    diff=__builtin_popcountll(value<<start_rest)-__builtin_popcountll(old_value<<start_rest);
    cdarray->ref(slate)+=diff;
    amount_1+=diff;
    if (slate!=find_slate(end+1)){
        this->_init_slate(slate+1);
        // create new search block and invert
        uint64_t bitm_next=~((-1ull)>>(start_rest-1));
        array[end/64] = (value>>next_rest)|(array[end/64]&(bitm_next));
        diff = __builtin_popcountll(value>>next_rest)-__builtin_popcountll(old_value>>next_rest);
        cdarray->ref(slate+1)+=diff;
        amount_1+=diff;
    }
}

void RSBitmap::make_static(){
    if(n()==0)
        return;
    // exactly one of these structures may be valid
    assert((this->cdarray==nullptr) != (this->rsarray==nullptr));
    // already static
    if(!this->cdarray)
        return;
    for(uint32_t counter=1; counter<=amount_slates; counter++){
        this->_init_slate(counter);
    }
    delete this->cdarray;
    this->cdarray=nullptr;
    assert(this->rsarray==nullptr);
    assert(this->array!=nullptr);
    this->rsarray = new rank9sel(this->array, blocks()*64);

    assert(this->cdarray==nullptr && this->rsarray!=nullptr);
}

uint64_t RSBitmap::rank(const uint64_t pos) const{
    SFLCHECK(pos!=0)
    if(pos>n() || amount_1==0)
        return amount_1;
    if(rsarray){
        return rsarray->rank(pos-1);
    }
    uint64_t rank=0, ipos=1, limiter=find_slate(pos), _size=size_slate*64;
    for(uint64_t counter=1; counter<limiter; counter++){
        rank+=cdarray->get(counter);
        ipos+=_size;
    }
    assert(ipos<=n());
    // should be at slate begin
    assert((ipos%_size)==1);
    for (; ipos<pos; ipos++){
        if(get(ipos))
            ++rank;
    }
    assert(rank<=amount_1);
    return rank;
}
uint64_t RSBitmap::select(const uint64_t rank) const{
    if(rank>=amount_1){
        return 0;
    }
    if(rsarray){
        return rsarray->select(rank)+1;
    }

    uint64_t _rank=rank+1, ipos=1, ranktmp, _size=size_slate*64;
    ranktmp = cdarray->get(find_slate(ipos));
    while(ranktmp<_rank){
        _rank-=ranktmp;
        ipos+=_size;
        ranktmp = cdarray->get(find_slate(ipos));
    }
    // should be at slate begin
    assert((ipos%_size)==1);
    assert(ipos%64==1);
    uint64_t block;
    while (_rank>=1){
        assert(is_pos_init(ipos));
        if (ipos%64==1){
            block = array[(ipos-1)/64];
        }
        if(block & (1ull << ((ipos-1)%64)))
            --_rank;
        ++ipos;
    }
    assert(ipos<=n()+1);
    return ipos-1;
}


uint64_t RSBitmap::next_one(const uint64_t pos) const{
    if(pos>n())
        return 0;
    if(rsarray){
        return this->select(this->rank(pos));
    }
    uint64_t ipos=pos, slate=find_slate(pos), _size=size_slate*64;
    assert(slate>0);


    while (ipos<=n() && !get(ipos)){
        assert(slate==find_slate(ipos));
        if(cdarray->get(slate)==0){
            // start: (ipos%_size)!=1
            // align to slate
            // 1 causes wrap to next slate (next to alignment)
            // -1 because slate ids begin with 1,
            ipos=1+_size*slate; // -1 replaced by moved ++slate statement
            ++slate;
            assert((ipos%_size)==1);
            assert(ipos>n() || slate==find_slate(ipos));
        } else {
            ++ipos;
            // new slate begins
            if (ipos%_size==1){
                ++slate;
            }
            assert(ipos>n() || slate==find_slate(ipos));
        }
    }
    if(ipos>n())
        return 0;
    return ipos;
}

uint64_t RSBitmap::rank_n(const uint64_t pos, const uint8_t nblock) const{
    return this->rank(pos*nblock);
};
uint64_t RSBitmap::select_n(const uint64_t rank, const uint8_t nblock) const{
    uint64_t sel = this->select(rank);
    if(sel==0)
        return 0;
    if ((sel-1)%nblock==0)
        return (sel-1)/nblock;
    else
        return ((sel-1)/nblock)+1;
}

uint64_t RSBitmap::next_one_n(const uint64_t pos, const uint8_t nblock) const{
    uint64_t sel = this->next_one(pos*nblock);
    if(sel==0)
        return 0;
    if ((sel-1)%nblock==0)
        return (sel-1)/nblock;
    else
        return ((sel-1)/nblock)+1;
}

bool ChoiceDictionary::insert(const uint64_t pos){
    // bound checking
    if (pos==0 || pos>capacity())
        return false;
    bool success = !array.get(pos);
    if (success){
        array.set(pos, true);
        if(pos<=_cached_position){
            _cached_position = pos;
        }
    }
    return success;
}

bool ChoiceDictionary::remove(const uint64_t pos) {
    // bound checking
    if (pos==0 || pos>capacity())
        return false;
    bool success = array.get(pos);
    if (success){
        array.set(pos, false);
    }
    return success;
}
uint64_t ChoiceDictionary::choice(void){
    // empty
    if (array.ones()==0)
        return 0;
    uint64_t pos;
    // don't use next element, if true return
    if(array.get(_cached_position)){
        pos = _cached_position;
    } else {
        pos = array.next_one(_cached_position);
        // next_one returned 0
        if (pos == _cached_position)
            return 0;
    }
    // skip checks, _size is enough
    array.set(pos, false);
    _cached_position = pos;
    return pos;
}

const std::vector<uint64_t> ChoiceDictionary::elements() const{
    std::vector<uint64_t> ret;
    for (uint64_t count=0; count<array.ones(); count++){
        ret.push_back(array.select(count));
    }
    return ret;
}
