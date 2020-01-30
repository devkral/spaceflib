/*! \file misc.hpp
    \author Alexander Kaftan
    \brief Structures for use in graphs
*/

#ifndef spacefmisc
#define spacefmisc

#include "commondefinitions.h"

#include <cstddef>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <type_traits>

class rank9sel;

//! Base type of ConstTimeArray
/*! \tparam T element type of the array
*/
template<typename T>
class BaseConstTimeArray{
protected:
public:
    virtual ~BaseConstTimeArray(){}
    //! handle uninitialized
    /*! \param pos position of element, 0 for default
    */
    virtual const T substitute_uninitialized(const uint64_t pos=0) const{
        throw;
    };
    //! max elements
    /*! \return maximal amount of elements
    */
    virtual uint64_t n() const = 0;
    //! Get element or default element, don't initialize
    /*! \param pos position of element
        \return element T
    */
    virtual const T get(const uint64_t pos) const = 0;
    //! Set element
    /*! \param pos position of element
        \param obj object
    */
    virtual void set(const uint64_t pos, const T obj) = 0;
};

//! Base type of ConstTimeArrayInitArrImpl
class ConstTimeInitArr{
    uint64_t n_=0;
public:
    //! Constructor
    /*! \param n amount of elements
    */
    ConstTimeInitArr(uint64_t n): n_(n){}
    //! Destructor
    virtual ~ConstTimeInitArr(){};
    //! Check if position is initialized or return position
    /*! \param pos position
        \return position or 0
    */
    virtual uint64_t get_pos(const uint64_t pos) const=0;
    //virtual uint64_t size() const=0;
    //! update initialization state of slot and return position
    virtual uint64_t update_arrays(const uint64_t pos)=0;
    //! explicit copy
    virtual ConstTimeInitArr* copy() const=0;
    //! current amount of elements
    /*! \return current amount of elements
    */
    virtual uint64_t size() const noexcept=0;
    //! maximal amount of elements
    /*! \return maximal amount of elements (=maximum position)
    */
    uint64_t n() const noexcept{
        return n_;
    }
};

//! Constant time allocation helper
/*! \tparam T Type holding array positions
*/
template<typename T>
class ConstTimeInitArrImpl: public ConstTimeInitArr{
    //! array initialization point->pos
    T *init_pointers1=nullptr;
    //! array pos->initialization point
    T *init_pointers2=nullptr;
    // Amount initialized fields
    uint64_t init_count=0;

    //! copy Constructor
    ConstTimeInitArrImpl(const ConstTimeInitArrImpl<T>& other):
    ConstTimeInitArrImpl(other.n()){
        init_count = other.init_count;
        // copy only size
        std::copy(other.init_pointers1, other.init_pointers1+size(), this->init_pointers1);
        // full size required
        std::copy(other.init_pointers2, other.init_pointers2+n(), this->init_pointers2);
    }
public:
    //! Construct n sized array
    ConstTimeInitArrImpl(uint64_t n) : ConstTimeInitArr(n){
        if (n>0){
            this->init_pointers1 = new T[n];
            this->init_pointers2 = new T[n];
        }
    }
    //! move Constructor
    ConstTimeInitArrImpl(ConstTimeInitArrImpl<T>&& other):
    ConstTimeInitArr(n()),
    init_count(other.init_count){
        assert(n()==other.n());
        assert(init_count==other.init_count);
        // move array pointers
        this->init_pointers1=other.init_pointers1;
        this->init_pointers2=other.init_pointers2;
        // invalidate old array pointers
        other.init_pointers1=nullptr;
        other.init_pointers2=nullptr;
    }
    ~ConstTimeInitArrImpl(){
        if(init_pointers1)
            delete[] init_pointers1;
        if(init_pointers2)
            delete[] init_pointers2;
    }
    //! explicit copy
    ConstTimeInitArr* copy() const{
        // const attribute of function prevents move constructor
        ConstTimeInitArrImpl* temp;
        temp = new ConstTimeInitArrImpl<T>(*this);
        return temp;
    }

    //! current amount of elements
    /*! \return current amount of elements
    */
    uint64_t size() const noexcept {
        return init_count;
    }

    //! Check if position is initialized or return position
    /*! \param pos position
        \return position or 0
    */
    uint64_t get_pos(const uint64_t pos) const {
        assert(pos>0);
        assert(pos<=n());
        T rarraypos = init_pointers2[pos-1];
        if (rarraypos<init_count && init_pointers1[rarraypos]==pos-1)
            return rarraypos+1;
        else{
            return 0;
        }
    }
    //! Update initialization state
    /*! \param pos position
    */
    uint64_t update_arrays(const uint64_t pos){
        assert(pos<=n());
        assert(pos>=1);
        uint64_t arrpos = get_pos(pos);
        if (arrpos==0){
            // save 0-based for optimal usage of storage and e.g. uint8_t
            init_pointers2[pos-1] = static_cast<T>(init_count);
            init_pointers1[init_count] = static_cast<T>(pos-1);
            this->init_count++;
            assert(init_count<=n());
            // +1 for 1-based output
            return init_count;
        } else {
            return arrpos;
        }
    }
};

//! Constant time array (unspecialized)
/*! \tparam T element type of the array
*/
template<typename T>
class ConstTimeArray: public BaseConstTimeArray<T>{
    //! array with objects
    T *array=nullptr;
    //! Initialization array
    ConstTimeInitArr *init=nullptr;
    //! defaultelement
    T defaultelement;
    // can be recursive, don't copy implicitly
    //! Copy constructor
    ConstTimeArray(const ConstTimeArray<T>& other):
    init(other.init->copy()),
    defaultelement(other.defaultelement){
        assert(sizeof(T)*init->n()>0);
        // allocate but don't initialize => constant time
        this->array = static_cast<T*>(::operator new(sizeof(T)*this->init->n()));
        // copy initialized
        std::copy(other.array, other.array+this->init->size(), this->array);
    }
public:
    //! Constructor
    /*! \param n amount of elements
        \param default_element default element which will be returned or used to initialize. Must be copyable.
    */
    ConstTimeArray(uint64_t n, const T &default_element): defaultelement(default_element) {
        // check if type is compatible. Bad if it uses non trivial destructors
        static_assert(std::is_trivially_destructible<T>::value, "Type is not trivial destructable");
        // Bad if it is not copyable
        static_assert(std::is_copy_constructible<T>::value, "Type is not copyable");
        // create pointerob_size in size of required integer type
        if (n==0){ // do nothing
        }else if (n<=UINT8_MAX)
            this->init = new ConstTimeInitArrImpl<uint8_t>(n);
        else if (n<=UINT16_MAX)
            this->init = new ConstTimeInitArrImpl<uint16_t>(n);
        else if (n<=UINT32_MAX)
            this->init = new ConstTimeInitArrImpl<uint32_t>(n);
        else
            this->init = new ConstTimeInitArrImpl<uint64_t>(n);

        assert(sizeof(T)*init->n()>0);
        // allocate but don't initialize => constant time
        this->array = static_cast<T*>(::operator new(sizeof(T)*n));
    }
    //! Move Constructor
    /*!
        steal data and pointers, invalidate pointer of the dying instance afterwards
    */
    ConstTimeArray(ConstTimeArray<T>&& other):
    defaultelement(other.defaultelement){
        // move array pointers
        this->array=other.array;
        this->init=other.init;
        // invalidate old array pointers
        other.array=nullptr;
        other.init=nullptr;
    }
    ~ConstTimeArray() noexcept{
        if(array){
            ::operator delete(this->array);
        }
        if(init)
            delete init;
    }

    //! Move assignment
    ConstTimeArray& operator=(ConstTimeArray &&other) noexcept{
        if (this != &other){
            defaultelement = other.defaultelement;
            // delete own
            if (this->array){
                ::operator delete(this->array);
            }
            if (this->init){
                delete this->init;
            }
            // move array pointers
            this->array=other.array;
            this->init=other.init;
            // invalidate old array pointers
            other.array=nullptr;
            other.init=nullptr;
        }
        return *this;
    }
    //! sustitute by defaultelement
    const T substitute_uninitialized(const uint64_t pos=0) const{
        return defaultelement;
    };
    //! explicit copy Array.
    /*!
        \return shallow copy
        \warning No recursive copy. Use with caution!
    */
    ConstTimeArray<T> copy() const{
        // const attribute of function prevents move constructor
        return ConstTimeArray<T>(*this);
    }
    //! max elements
    /*! \return maximal amount of elements
    */
    uint64_t n() const noexcept{
        return this->init->n();
    }

    //! Set element
    /*! \param pos position of element
        \param obj object
    */
    void set(const uint64_t pos, const T obj){
        // checks here are important as init is assert only
        SFLCHECK(pos>0)
        SFLCHECK(pos<=n())
        // use 0 based array
        array[this->init->update_arrays(pos)-1] = obj;
    }
    //! Get element or default element, don't initialize
    /*! \param pos position of element
        \return element T
    */
    const T get(const uint64_t pos) const {
        // checks here are important as init is assert only
        SFLCHECK(pos>0)
        if(pos>n())
            return substitute_uninitialized(pos);
        uint64_t arrpos = this->init->get_pos(pos);
        if (arrpos>0)
            return array[arrpos-1];
        else{
            return substitute_uninitialized(pos);
        }
    }
    //! Return reference to element or initialize with default element and return reference
    /*! \param pos position of element
        \return element T
        \warning not available for bool or subbyte arrays
    */
    T &ref(const uint64_t pos) {
        // checks here are important as init is assert only
        SFLCHECK(pos>0)
        SFLCHECK(pos<=n())
        uint64_t arrpos = this->init->get_pos(pos);
        if (arrpos!=0)
            return array[arrpos-1];
        else{
            arrpos = this->init->update_arrays(pos);
            // use 0 based array
            array[arrpos-1] = substitute_uninitialized(pos);
            return array[arrpos-1];
        }
    }
    //! Check if position is initialized
    /*! \param pos position
        \return state
        \warning not available for bool or subbyte arrays
    */
    bool is_init(const uint64_t pos) const {
        // checks here are important as init is assert only
        SFLCHECK(pos>0)
        SFLCHECK(pos<=n())
        return this->init->get_pos(pos)!=0;
    }
};

//! A robust bitmap with succinct operations
/*! \class RSBitmap
    Robust succinct Bitmap.
*/
class RSBitmap{
    //! data array
    uint64_t *array=nullptr;
    //! amount of bits
    uint64_t nbits_;
    //! amount of blocks
    uint64_t blocks_;
    //! size of a slate
    uint64_t size_slate=0;
    //! required amount of slates
    uint64_t amount_slates=0;
    //! Choice dictionary array
    ConstTimeArray<uint32_t>* cdarray=nullptr;
    //! speed up ranks select
    rank9sel* rsarray=nullptr;
    //! amount of ones
    size_t amount_1=0;
    //! init slates conditionally
    void _init_slate(const uint64_t slate);
    // move constructor should be used. To force this, make CopyConstructor private
    //! Copy Constructor
    RSBitmap(const RSBitmap&);
protected:
public:
    //! Move Constructor
    RSBitmap(RSBitmap&&);
    //! Move assignment
    RSBitmap& operator=(RSBitmap&&) noexcept;

    //! Constructor
    /*! \param nbits Number of bits
    */
    RSBitmap(uint64_t nbits);
    //! Destructor of RSBitmap
    ~RSBitmap();
    //! explicit copy RSBitmap.
    /*!
        \return copy
    */
    inline RSBitmap copy() const{
        // const attribute of function prevents move constructor
        return RSBitmap(*this);
    }
    //! size in bits
    /*! \return amount of bits
    */
    inline uint64_t n() const noexcept{return nbits_;}
    //! size in blocks
    /*! \return amount of blocks
    */
    inline uint64_t blocks() const noexcept{return blocks_;}
    //! calculate amount of required blocks from pos/nbits
    /*! \param pos position
        \return required amount of blocks or 0
    */
    static uint64_t calc_blocks(const uint64_t pos) noexcept;
    //! find slate corresponding to position
    /*! \param pos position
        \return slate position
    */
    inline uint64_t find_slate(const uint64_t pos) const{
        if(pos==0 || pos>n())
            return 0;
        const uint64_t _size = size_slate*64;
        // find_slate is called in get_n, this means cdarray not neccessary available
        assert(!cdarray || cdarray->n()>=(pos-1)/_size+1);
        return (pos-1)/_size+1;
    }
    //! return amount of 1 in array
    /*! \return amount of 1 set
    */
    inline uint64_t ones(void) const noexcept {
        return amount_1;
    }
    //! improves efficiency of rank, select but make array static
    void make_static();
    //! check if RSBitmap is static
    /*!
        \return is RSBitmap static
    */
    inline bool is_static() const noexcept {
        if (this->rsarray){
            return true;
        }
        return false;
    }
    //! set bit
    /*! \param pos position of bit
        \param state bit state
    */
    void set(const uint64_t pos, const bool state);
    //! flip bit
    /*! \param pos position of bit
    */
    void flip(const uint64_t pos);
    //! get bit
    /*! \param pos position of bit
        \return bit state
    */
    bool get(const uint64_t pos) const;
    //! use bitmap like an array with n-bit width blocks (get block)
    /*! \param pos position of bit
        \param nblock block size
        \return bit state
        requires nblock is divisor of n()
    */
    uint64_t get_n(const uint64_t pos, const uint8_t nblock) const;
    //! use bitmap like an array with n-bit width blocks (set block)
    /*! \param pos position of bit
        \param nblock block size
        \param value value to set
        requires nblock is divisor of n()
    */
    void set_n(const uint64_t pos, const uint8_t nblock, const uint64_t value);
    //! is slate corresponding to position initialized
    /*!
        \param pos position
        \return initialization state
    */
    inline bool is_pos_init(const uint64_t pos) const{
        // if cdarray does not exist every bit is initialized
        if(!cdarray)
            return true;
        return cdarray->is_init(find_slate(pos));
    }
    //! compute rank
    /*! \param pos position of bit
        \return 1 till position or throws exception
    */
    uint64_t rank(const uint64_t pos) const;
    //! compute select
    /*! \param rank rank
        \return pos of next 1 or 0 (error)
    */
    uint64_t select(const uint64_t rank) const;

    //! compute rank
    /*! \param pos position of bit
        \param nblock block size
        \return 1 till position or throws exception
    */
    uint64_t rank_n(const uint64_t pos, const uint8_t nblock) const;
    //! compute select
    /*! \param rank rank
        \param nblock block size
        \return pos of next 1 or 0 (error)
    */
    uint64_t select_n(const uint64_t rank, const uint8_t nblock) const;

    //! next 1
    /*! \param pos position
        \return next position of 1 or 0 (error)
        speed up select next 1
    */
    uint64_t next_one(const uint64_t pos) const;
    //! next 1
    /*! \param pos position
        \param nblock block size
        \return next position of 1 or pos if 1 or 0 (error)
        speed up select next 1
    */
    uint64_t next_one_n(const uint64_t pos, const uint8_t nblock) const;
    //! cond
    //! Debug method, returns slate size
    uint64_t _get_size_slate() const noexcept{ return size_slate;}
    //! endcond
};

//! helper for multi bit access
/*!
    \tparam size amount of bits (possible range 1-64)
*/
template<size_t size>
struct array_multi_bit{};


//! Constant time array (multi bit specialization)
template<size_t size>
class ConstTimeArray<array_multi_bit<size>> : public BaseConstTimeArray<uint64_t> {
protected:
    //! multibit container
    RSBitmap bitmap;
    //! copy constructor, only internal
    ConstTimeArray(const ConstTimeArray<array_multi_bit<size>>& other): bitmap(other.bitmap.copy()){}
public:
    //! sustitute by 0
    const uint64_t substitute_uninitialized(const uint64_t pos=0) const{
        return 0;
    };
    // ignore default_element
    //! Construct pseudo ConstTimeArray array
    ConstTimeArray(uint64_t n) : bitmap(n*size){}
    // ignore default_element
    //! Construct pseudo ConstTimeArray array
    template<typename z>
    ConstTimeArray(uint64_t n, const z) : ConstTimeArray(n){}
    //! try to move
    ConstTimeArray(ConstTimeArray<array_multi_bit<size>>&& other):
    bitmap(std::move(other.bitmap)){}
    //! Move assignment
    ConstTimeArray& operator=(ConstTimeArray &&other) noexcept{
        if (this != &other){
            bitmap=std::move(other.bitmap);
        }
        return *this;
    }
    //! explicit copy Array.
    /*!
        \return copy
    */
    ConstTimeArray<array_multi_bit<size>> copy() const{
        return ConstTimeArray<array_multi_bit<size>>(*this);
    }
    //! max elements
    /*! \return maximal amount of elements
    */
    uint64_t n() const noexcept{
        return bitmap.n()/size;
    }

    //! return amount of 1 in array
    /*! \return amount of 1 set
        Note: not the amount of elements in array
    */
    uint64_t ones(void) const noexcept{
        return bitmap.ones();
    };
    //! next 1
    /*! \param pos position
        \return next position of 1 or pos if 1 or 0 (error)
        speed up select next 1
    */
    uint64_t next_one(const uint64_t pos) const{
        return bitmap.next_one_n(pos, size);
    }
    //! Get element or default element, don't initialize
    /*! \param pos position of element
        \return element T
    */
    const uint64_t get(const uint64_t pos) const{
        if (pos>n())
            return substitute_uninitialized(pos);
        return bitmap.get_n(pos, size);
    }
    //! Set element
    /*! \param pos position of element
        \param obj object
    */
    void set(const uint64_t pos, const uint64_t obj){
        bitmap.set_n(pos, size, obj);
    }
    //! compute rank
    /*! \param pos position of bit
        \return 1 till position or throws exception
    */
    uint64_t rank(const uint64_t pos) const{
        return bitmap.rank_n(pos, size);
    }
    //! compute select
    /*! \param rank rank
        \return pos of next 1 or 0 (error)
    */
    uint64_t select(const uint64_t rank) const{
        return bitmap.select_n(rank, size);
    }
    //! improves efficiency of rank, select but make array static
    void make_static(){
        bitmap.make_static();
    }
    //! check if RSBitmap is static
    /*!
        \return is RSBitmap static
    */
    bool is_static() const noexcept {
        return bitmap.is_static();
    }
};


//! Constant time array (boolean specialization)
template<>
class ConstTimeArray<bool> : public BaseConstTimeArray<bool> {
protected:
    //! boolean container
    RSBitmap bitmap;
    //! copy constructor, only internal
    ConstTimeArray(const ConstTimeArray<bool>& other): bitmap(other.bitmap.copy()){}
public:
    //! sustitute by false
    const bool substitute_uninitialized(const uint64_t pos=0) const{
        return false;
    };
    // ignore default_element
    //! Construct pseudo ConstTimeArray array
    ConstTimeArray(uint64_t n) : bitmap(n){}
    // ignore default_element
    //! Construct pseudo ConstTimeArray array
    template<typename z>
    ConstTimeArray(uint64_t n, const z) : ConstTimeArray(n){}
    //! try to move
    ConstTimeArray(ConstTimeArray<bool>&& other):
    bitmap(std::move(other.bitmap)){}
    //! Move assignment
    ConstTimeArray& operator=(ConstTimeArray &&other) noexcept{
        if (this != &other){
            bitmap=std::move(other.bitmap);
        }
        return *this;
    }
    //! explicit copy Array.
    /*!
        \return copy
    */
    ConstTimeArray<bool> copy() const{
        return ConstTimeArray<bool>(*this);
    }
    //! max elements
    /*! \return maximal amount of elements
    */
    uint64_t n() const noexcept{
        return bitmap.n();
    }
    //! return amount of 1 in array
    /*! \return amount of 1 set
        Note: in this case this is the whole amount of elements in the array
    */
    uint64_t ones(void) const noexcept{
        return bitmap.ones();
    };
    //! next 1
    /*! \param pos position
        \return next position of 1 or pos if 1 or 0 (error)
        speed up select next 1
    */
    uint64_t next_one(const uint64_t pos) const{
        return bitmap.next_one(pos);
    }
    //! Get element or default element, don't initialize
    /*! \param pos position of element
        \return element T
    */
    const bool get(const uint64_t pos) const{
        if (pos>n())
            return substitute_uninitialized(pos);
        return bitmap.get(pos);
    }
    //! Set element
    /*! \param pos position of element
        \param obj object
    */
    void set(const uint64_t pos, bool obj){
        bitmap.set(pos, obj);
    }

    //! compute rank
    /*! \param pos position of bit
        \return 1 till position or throws exception
    */
    uint64_t rank(const uint64_t pos) const{
        return bitmap.rank(pos);
    }
    //! compute select
    /*! \param rank rank
        \return pos of next 1 or 0 (error)
    */
    uint64_t select(const uint64_t rank) const{
        return bitmap.select(rank);
    }
    //! improves efficiency of rank, select but make array static
    void make_static(){
        bitmap.make_static();
    }
    //! check if RSBitmap is static
    /*!
        \return is RSBitmap static
    */
    bool is_static() const noexcept{
        return bitmap.is_static();
    }
};


//! Segmented Array
/*!
    \tparam parameters type pack for which ConstTimeArrays are generated
    examples for defaultelement for spezializations:
    boolean: SegmentedArray<bool, bool>(false, true); //false, true are ignored
    array_multi_bit: SegmentedArray<array_multi_bit<2>, array_multi_bit<3>>(array_multi_bit<2>(), array_multi_bit<3>())
    mixed example: SegmentedArray<bool, array_multi_bit<3>>(true, array_multi_bit<3>())
*/
template<typename... parameters>
class SegmentedArray
{
    uint64_t array_size_, amount_;
    RSBitmap segments_;
    RSBitmap notempty_;

    template <class Tuple, std::size_t... Counter>
    constexpr auto copy_arrays(Tuple &t, std::index_sequence<Counter...>) {
        return std::make_tuple(std::move(std::get<Counter>(t).copy())...);
    }


    //! Copy Constructor
    SegmentedArray(const SegmentedArray& other):
    array_size_(other.array_size_),
    amount_(other.amount_),
    segments_(other.segments_.copy()),
    notempty_(other.notempty_.copy()),
    arrays(copy_arrays(other.arrays, std::index_sequence_for<parameters...>{})){}

protected:
    //! calculate amount of segments
    /*! \param start segment sizes iterator begin
        \param end segment sizes iterator end
        \return amount of segments
    */
    template<typename Iterator>
    static uint64_t calc_amount(Iterator start, Iterator end){
        uint64_t amount=0;
        for(auto it=start; it!=end; it++){
            amount+=1;
        }
        return amount;
    }
    //! calculate complete array size
    /*! \param start segment sizes iterator begin
        \param end segment sizes iterator end
        \return minimal arraysize
    */
    template<typename Iterator>
    static uint64_t calc_array_size(Iterator start, Iterator end){
        uint64_t size=0;
        for(auto it=start; it!=end; it++){
            size+=*it;
        }
        return size;
    }

public:
    //! container type
    typedef std::tuple<ConstTimeArray<parameters>...> SegArrays;
    //! container for arrays
    SegArrays arrays;
    //! constructor
    /*!
        \param defaultelement parameter pack with defaultelements
        \param start segment sizes iterator begin
        \param end segment sizes iterator end
        examples for defaultelement for spezializations:
        boolean: SegmentedArray<bool, bool>(segments, false, true); //false, true are ignored
        array_multi_bit: SegmentedArray<array_multi_bit<2>, array_multi_bit<3>>(segments, array_multi_bit<2>(), array_multi_bit<3>());
        mixed example: SegmentedArray<uint32_t, bool, array_multi_bit<3>>(segments, 0, true, array_multi_bit<3>());
    */
    template<typename Iterator, typename... defaults>
    SegmentedArray(Iterator start, Iterator end, defaults... defaultelement):
    array_size_(calc_array_size(start, end)),
    amount_(calc_amount(start, end)),
    segments_(array_size_+1),
    notempty_(amount_+1),
    arrays(ConstTimeArray<parameters>(array_size_, defaultelement)...)
    {
        uint64_t counter=1, segment_begin=1;
        for(auto it=start; it!=end; it++){
            if(*it!=0){
                this->notempty_.set(counter, true);
                this->segments_.set(segment_begin, true);
                segment_begin+=*it;
            }
            counter++;
        }
        // stopper bits (required for segment_size)
        this->notempty_.set(counter, true);
        this->segments_.set(segment_begin, true);
        this->notempty_.make_static();
        this->segments_.make_static();
    }
    //! constructor
    /*
        \param defaultelement parameter pack with defaultelements
        \param segments initializer_list({}) with segment sizes
        examples for defaultelement for spezializations:
        boolean: SegmentedArray<bool, bool>(false, true); //false, true are ignored
        array_multi_bit: SegmentedArray<array_multi_bit<2>, array_multi_bit<3>>(array_multi_bit<2>(), array_multi_bit<3>())
        mixed example: SegmentedArray<bool, array_multi_bit<3>>(true, array_multi_bit<3>())

    template<typename... defaults>
    SegmentedArray(std::initializer_list<uint64_t> segments, defaults... defaultelement): SegmentedArray(std::vector<uint64_t>(segments), defaultelement...){}
    */

    //! Move Constructor
    SegmentedArray(SegmentedArray&& other):
    array_size_(other.array_size_),
    amount_(other.amount_),
    segments_(std::move(other.segments_)),
    notempty_(std::move(other.notempty_)),
    arrays(std::move(other.arrays)){}

    //! destructor
    ~SegmentedArray() = default;
    //! allow move assignment operation
    SegmentedArray& operator=(SegmentedArray&&) = default;

    //! explicit copy SegmentedArray.
    /*!
        \return copy of SegmentedArray and underlying arrays
        Arrays must implement copy method.
    */
    SegmentedArray copy() const{
        // const attribute of function prevents move constructor
        return SegmentedArray(*this);
    }

    //! get_pos
    /*! \param segment segment in array
        \param pos position in segment
    */
    const uint64_t get_pos(const uint64_t segment, const uint64_t pos) const{
        SFLCHECK(segment>0)
        SFLCHECK(pos>0)
        SFLCHECK(segment<=segments());
        // check if empty
        if (!this->notempty_.get(segment))
            return 0;
        uint64_t seg_beg = segments_.select(this->notempty_.rank(segment));
        // no error should happen here
        assert(seg_beg>0);
        // check segment size, the efficient way. Nearly no overhead so use SFLCHECK
        if(pos>segments_.next_one(seg_beg+1)-seg_beg)
            return 0;
        seg_beg+=pos-1;
        // internal bound check
        assert(seg_beg<=size());
        return seg_beg;
    }
    //! reset array
    /*!
        \tparam SelArr select array
    */
    template<std::size_t SelArr>
    void reset(){
        using arrtype = typename std::tuple_element<SelArr, SegArrays>::type;
        std::get<SelArr>(this->arrays) = arrtype(size());
    }
    //! set item
    /*!
        \tparam SelArr select array
        \param arrpos array position of item
        \param obj object to store
    */
    template<size_t SelArr, typename First>
    void set(const uint64_t arrpos, const First obj){
        // ignore invalid
        if (arrpos>0)
            std::get<SelArr>(this->arrays).set(arrpos, std::move(obj));
    }
    //! get object
    /*!
        \tparam SelArr select array
        \param arrpos array position of item
        \return bit state
    */
    // get type by substitute_uninitialized
    template<size_t SelArr>
    auto get(const uint64_t arrpos) const -> decltype(std::get<SelArr>(this->arrays).substitute_uninitialized(arrpos)){
        if(arrpos==0){
            return std::get<SelArr>(this->arrays).substitute_uninitialized(arrpos);
        }
        return std::get<SelArr>(this->arrays).get(arrpos);
    }
    //! array size
    /*! \return the amount of elements in each array the structure maintains (real size)
    */
    const uint64_t size() const noexcept{
        return array_size_;
    }
    //! amount of segments
    /*! \return the amount of segments
    */
    const uint64_t segments() const noexcept{
        return amount_;
    }
    //! segment size
    /* \param segment segment
       \return segment size
    */
    const uint64_t segment_size(const uint64_t segment) const{
        SFLCHECK(segment>0)
        SFLCHECK(segment<=segments())
        if (!notempty_.get(segment))
            return 0;
        // cannot use get_pos here, has assert, that fake segment is never reached
        // also some recursion problems
        uint64_t rank_small = this->notempty_.rank(segment);
        // not empty so next_one finds the size
        uint64_t seg_beg = segments_.select(rank_small);
        uint64_t ret = segments_.next_one(seg_beg+1);
        assert(ret>0);
        assert(ret-seg_beg>0);
        return ret-seg_beg;
    }
    //! select 1 in full array
    /*! \tparam ObjType ObjectType with next_one method
        \param obj object for calculation
        \param segment segment
        \param pos position
        \param seg_begin hint where segment begins, default: 0 auto, WARNING: must be correct
        \return 1-based position in array where item can be found or 0
    */
    template<typename ObjType>
    const uint64_t select_array_pos(const ObjType &obj, const uint64_t segment, const uint64_t pos, uint64_t seg_begin=0) const {
        if (!notempty_.get(segment))
            return 0;
        if (seg_begin==0)
            seg_begin = get_pos(segment, 1);
        // select next including current
        // using select(rank(seg_begin+pos-1)) causes bad performance
        uint64_t ret = obj.next_one(seg_begin+pos-1);
        // over max rank
        if(ret==0)
            return 0;
        // 0 based checks
        assert(ret>=seg_begin);
        // next_one is here segment_size+seg_begin, because notempty is queried already
        // speed optimization
        if (ret>=segments_.next_one(seg_begin+1))
            return 0;
        return ret;
    }
    //! select 1 in segment from position
    /*! \tparam ObjType ObjectType with rank, select method to input
        \param obj object for calculation
        \param segment segment
        \param pos position
        \param seg_begin hint where segment begins, default: 0 auto, WARNING: must be correct
        \return 1-based position in segment where item can be found or 0
    */
    template<typename ObjType>
    const uint64_t select_segment_pos(const ObjType &obj, const uint64_t segment, const uint64_t pos, uint64_t seg_begin=0) const {
        if (!notempty_.get(segment))
            return 0;
        if (seg_begin==0)
            seg_begin = get_pos(segment, 1);
        else
            assert(seg_begin==get_pos(segment, 1));
        uint64_t ret = select_array_pos<ObjType>(obj, segment, pos, seg_begin);
        // over max rank or segment size
        if(ret==0)
            return 0;
        return ret+1-seg_begin;
    }
    //! how many ones are in a segment
    /*! \tparam ObjType ObjectType with rank, select method to input
        \param obj object for calculation
        \param segment segment
        \return amount of ones
    */
    template<typename ObjType>
    const uint64_t ones_segment(const ObjType &obj, const uint64_t segment) const {
        if (!notempty_.get(segment))
            return 0;
        uint64_t seg_begin = get_pos(segment, 1);
        // rank lists ones before segment => +1
        // next_one returns difference => -1
        // => 0
        return obj.rank(segments_.next_one(seg_begin+1))-obj.rank(seg_begin);
    }
    //! get empty segments before element
    /*! \param segment count empty before segment
        \return amount of empty segments before segment
    */
    const uint64_t get_empty(const uint64_t segment) const{
        SFLCHECK(segment>0)
        SFLCHECK(segment<=segments())
        return segment-notempty_.rank(segment);
    }
    //! get all empty segments
    /*!
        \return amount of empty segments
    */
    const uint64_t all_empty() const noexcept{
        return notempty_.n()-notempty_.ones();
    }

};



//! An efficient stack with fix size
/*! \class SpinStack
    \tparam T type which will be saved in the stack
    static Stack which uses a spinpointer for efficiency
*/
template<class T>
class SpinStack{
    uint64_t limitter=0, pointer=0;
    bool empty_=true;
    T *array=nullptr;
public:
    //! maximal stack size
    /*! \var n
    */
    const uint64_t n;
    //! Constructor
    /*! \param size maximal stack size
    */
    SpinStack(const uint64_t size): n(size){
        // check if type is compatible. Bad if it uses non trivial destructors
        static_assert(std::is_trivially_destructible<T>::value, "Type is not trivial destructable");
        assert(this->n>0);
        // allocate but don't initialize
        this->array = static_cast<T*>(::operator new(sizeof(T)*this->n));
    }
    SpinStack(const SpinStack& that) = delete;
    // steal pointer and invalidate pointer of dying instance
    //! Move Constructor
    SpinStack(SpinStack&& other) noexcept:
        limitter(other.limitter),
        pointer(other.pointer),
        empty_(other.empty_),
        n(other.n){
        this->array=other.array;
        other.array=nullptr;
    }
    ~SpinStack() noexcept{
        if(this->array){
            ::operator delete(this->array);
        }
    }
    //! Pop last element from stack
    /*! \return element T
        \warning always check empty() before pop
        Pop last element from stack.
    */
    T pop(){
        SFLCHECK(!empty())
        pointer = (n+pointer-1)%n;
        if (limitter==pointer)
            empty_=true;
        return array[pointer];
    }
    //! Peek last element on stack
    /*! \return element T
        \warning always check empty() before peek
        Peek last element on stack.
    */
    T peek(){
        SFLCHECK(!empty())
        return array[(n+pointer-1)%n];
    }
    //! Push element on the stack
    /*! \warning always check full() before push
        Push element on stack (STACK).
    */
    void push_top(T obj){
        SFLCHECK(!full())
        array[pointer] = obj;
        pointer=(pointer+1)%n;
        empty_=false;
    }
    //! Push element on the bottom of the stack
    /*! \warning always check full() before push
        Push element on bottom (FIFO).
    */
    void push_bottom(T obj){
        SFLCHECK(!full())
        limitter = (n+limitter-1)%n;
        array[limitter] = obj;
        empty_=false;
    }
    //! Drop number of elements at the front of the stac
    /*! \param num amount of elements at the front of the stack which will be dropped
        \return number of elements really dropped
        Drop num deepest elements from stack
    */
    uint64_t drop_front(uint64_t num){
        uint64_t max_drop = std::min(size(), num);
        limitter = (limitter+max_drop)%n;
        if (limitter==pointer)
            empty_=true;
        return max_drop;
    }
    //! size of stack
    /*! \return amount of elements on the stack
        Get current size of stack
    */
    uint64_t size(void) const noexcept{
        if (empty()) {
            return 0;
        } else {
            if (limitter >= pointer) {
                return pointer+(n-limitter);
            }
            else {
                return pointer-limitter;
            }
        }
    }
    //! is stack empty
    /*! \return if stack is empty
        Get empty state from stack
    */
    bool empty(void) const noexcept{
        return empty_;
    }
    //! is stack full
    /*! \return if stack is full
        Get full state from stack
    */
    bool full(void) const noexcept{
        return size()>=n;
    }
};


//! A choice dictionary basing on RSBitmap
/*! \class ChoiceDictionary
*/
class ChoiceDictionary{
    RSBitmap array;
    uint64_t _cached_position=1;
    ChoiceDictionary(const ChoiceDictionary& that) = default;
public:
    //! Constructor
    ChoiceDictionary(const uint64_t capacity) : array(capacity){};
    //! Move Constructor
    ChoiceDictionary(ChoiceDictionary&& other) noexcept :
        array(std::move(other.array)), _cached_position(other._cached_position){};
    //! maximal capacity
    /*! \var capacity
    */
    uint64_t capacity () const noexcept {return array.n();}
    //! insert
    /*! \param elem number to insert
        \return success of operation
    */
    bool insert(const uint64_t elem);
    //! remove
    /*! \param elem number to remove
        \return success of operation
    */
    bool remove(const uint64_t elem);
    //! contains
    /*! \param elem number to check
        \return true if found, false otherwise (or in error case)
        Does ChoiceDictionary contains element
    */
    bool contains(const uint64_t elem) const {return array.get(elem);}
    //! random element or 0 (error)
    /*! \return random element in ChoiceDictionary
        return random element and remove it
    */
    uint64_t choice();
    //! size
    /*! \return size
    */
    uint64_t size() const noexcept { return array.ones();}
    //! elements
    /*! \return elements in ChoiceDictionary
        return all elements in ChoiceDictionary
    */
    const std::vector<uint64_t> elements() const;
};


#endif
