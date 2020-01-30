
#include "thirdparty/Catch2/include/catch.hpp"

#include "../src/misc.hpp"
#include <string>
#include <vector>

TEST_CASE( "ConstTimeArray") {
    SECTION("Basic"){
        ConstTimeArray<uint32_t> t_array(12, 1);
        REQUIRE(t_array.n()==12);
        REQUIRE(t_array.get(1)==1);
        t_array.set(1, 6);
        REQUIRE(t_array.get(9923)==1);
        REQUIRE(t_array.get(1)==6);
        t_array.ref(12) = 74722;
        REQUIRE(t_array.get(12)==74722);
        REQUIRE(t_array.get(3312)==1);
        // test compatibility to move, doesn't compile or crash otherwise
        ConstTimeArray<uint32_t> array2 = std::move(t_array);
        REQUIRE(array2.n()==12);
        REQUIRE(array2.get(3312)==1);
        REQUIRE(array2.get(12)==74722);
        REQUIRE(array2.get(1)==6);
    }
    SECTION("Bool"){
        ConstTimeArray<bool> t_array(12, true);
        REQUIRE(t_array.n()==12);
        REQUIRE(t_array.get(1)==false);
        t_array.set(1, true);
        REQUIRE(t_array.get(9923)==false);
        REQUIRE(t_array.get(1)==true);
        // test compatibility to move, doesn't compile or crash otherwise
        ConstTimeArray<bool> array2 = std::move(t_array);
        REQUIRE(array2.n()==12);
        REQUIRE(array2.get(3312)==false);
        REQUIRE(array2.get(12)==false);
        REQUIRE(array2.get(1)==true);
    }
    SECTION("multibit"){
        ConstTimeArray<array_multi_bit<5>> t_array(12, array_multi_bit<5>());
        REQUIRE(t_array.get(1)==0);
        REQUIRE(t_array.n()==12);
        t_array.set(1, 6);
        t_array.set(3, 2);
        REQUIRE(t_array.get(9923)==0);
        REQUIRE(t_array.get(1)==6);
        REQUIRE(t_array.get(3312)==0);
        // test compatibility to move, doesn't compile or crash otherwise
        ConstTimeArray<array_multi_bit<5>> array2 = std::move(t_array);
        REQUIRE(array2.n()==12);
        REQUIRE(array2.get(3312)==0);
        REQUIRE(array2.get(1)==6);
        REQUIRE(array2.get(3)==2);
    }
}

TEST_CASE( "RSBitmap operations", "[succinct]" ) {
    SECTION("Construct"){
        RSBitmap rsb(200);
        REQUIRE(rsb.n()==200);
    }

    SECTION("Edge cases"){
        REQUIRE_NOTHROW(RSBitmap(0));
        RSBitmap rsb(11);
        REQUIRE_NOTHROW(rsb.set(2, true));
        REQUIRE(rsb.n()==11);
        std::vector<RSBitmap> array;
        for (int i=1; i<=10; i++){
            CAPTURE( i );
            array.push_back(RSBitmap(i*50));
        }
    }


    SECTION("n blocks"){
        RSBitmap rsb(66);
        REQUIRE(rsb.n()==66);
        REQUIRE_NOTHROW(rsb.set_n(1, 3, 5));
        REQUIRE(rsb.get_n(1, 3)==5);
        REQUIRE(rsb.get(1)==true);
        REQUIRE(rsb.get(2)==false);
        REQUIRE(rsb.get(3)==true);
        REQUIRE_NOTHROW(rsb.set_n(1, 3, 4));
        REQUIRE(rsb.get_n(1, 3)==4);
        REQUIRE_NOTHROW(rsb.set_n(22, 3, 5));
        REQUIRE(rsb.get_n(22, 3)==5);
        REQUIRE(rsb.get(64)==true);
        REQUIRE(rsb.get(65)==false);
        REQUIRE(rsb.get(66)==true);
        REQUIRE(rsb.get_n(22, 3)==5);
        REQUIRE_NOTHROW(rsb.set_n(22, 3, 4));
        REQUIRE(rsb.get(64)==false);
        REQUIRE(rsb.get(65)==false);
        REQUIRE(rsb.get(66)==true);
        REQUIRE(rsb.get_n(22, 3)==4);
    }

    SECTION("Bitmap operations"){
        RSBitmap rsb(200);
        rsb.set(1, true);
        REQUIRE(rsb.get(1)==true);
        rsb.set(4, true);
        REQUIRE(rsb.get(4)==true);
        for (int i=5; i<200; i++){
            CAPTURE( rsb.n() );
            CAPTURE( i );
            REQUIRE(rsb.get(i)==false);
        }
        rsb.set(11, true);
        for (int i=5; i<11; i++){
            CAPTURE( i );
            REQUIRE(rsb.get(i)==false);
        }
        REQUIRE(rsb.get(200)==false);
        rsb.set(200, true);
        REQUIRE(rsb.get(200)==true);
        rsb.flip(200);
        REQUIRE(rsb.get(200)==false);
        rsb.flip(200);
        REQUIRE(rsb.get(200)==true);

        REQUIRE(rsb.get(100)==false);
        rsb.set(100, true);
        REQUIRE(rsb.get(100)==true);
        rsb.flip(100);
        REQUIRE(rsb.get(100)==false);
        rsb.flip(100);
        REQUIRE(rsb.get(100)==true);

        REQUIRE(rsb.get(0)==false);
        REQUIRE_NOTHROW(rsb.flip(0));
        REQUIRE_NOTHROW(rsb.set(0, true));

        REQUIRE(rsb.get(201)==false);
        REQUIRE_NOTHROW(rsb.flip(201));
        REQUIRE_NOTHROW(rsb.set(201, true));

        REQUIRE(rsb.get(400)==false);
        REQUIRE_NOTHROW(rsb.flip(400));
        REQUIRE_NOTHROW(rsb.set(400, true));

        // test compatibility to move, doesn't compile or crash otherwise
        RSBitmap rsbmove = std::move(rsb);
        REQUIRE(rsbmove.get(200)==true);
        REQUIRE(rsbmove.get(100)==true);
    }

    SECTION("Succinct"){
        RSBitmap rsb2(200);
        rsb2.set(1,true);
        rsb2.set(3,true);
        rsb2.set(11,true);
        rsb2.set(62,true);
        rsb2.set(63,true);
        rsb2.set(64,true);
        rsb2.set(126,true);
        rsb2.set(127,true);
        rsb2.set(199,true);
        rsb2.set(200,true);
        // rank
        // first two are invalid
        CHECK_THROWS(rsb2.rank(0));
        REQUIRE(rsb2.rank(2003)==10);
        REQUIRE(rsb2.rank(1)==0);
        REQUIRE(rsb2.rank(2)==1);
        REQUIRE(rsb2.rank_n(2, 1)==1);
        REQUIRE(rsb2.rank_n(1, 2)==1);
        REQUIRE(rsb2.rank(3)==1);
        REQUIRE(rsb2.rank_n(3, 1)==1);
        REQUIRE(rsb2.rank_n(2, 2)==2);
        REQUIRE(rsb2.rank(63)==4);
        REQUIRE(rsb2.rank(64)==5);
        REQUIRE(rsb2.rank_n(32, 2)==5);
        REQUIRE(rsb2.rank(65)==6);
        REQUIRE(rsb2.rank(126)==6);
        REQUIRE(rsb2.rank(127)==7);
        REQUIRE(rsb2.rank(128)==8);
        REQUIRE(rsb2.rank(200)==9);
        REQUIRE(rsb2.rank(201)==10);
        // check if the answer did change
        REQUIRE(rsb2.rank(65)==6);

        // select
        REQUIRE(rsb2.select(0)==1);
        REQUIRE(rsb2.select(rsb2.rank(1))==1);
        REQUIRE(rsb2.select_n(rsb2.rank_n(1, 2), 2)==1);
        REQUIRE(rsb2.select(1)==3);
        REQUIRE(rsb2.select(2)==11);
        REQUIRE(rsb2.select(rsb2.rank(62))==62);
        REQUIRE(rsb2.select_n(rsb2.rank_n(32, 2), 2)==32);
        REQUIRE(rsb2.select(7)==127);
        REQUIRE(rsb2.select(9)==200);
        REQUIRE(rsb2.select_n(rsb2.rank_n(100, 2), 2)==100);
        REQUIRE(rsb2.select(10)==0);
        REQUIRE(rsb2.select(11)==0);
        REQUIRE(rsb2.select(124)==0);
        REQUIRE(rsb2.select_n(124, 2)==0);
        // check if the answer did change
        REQUIRE(rsb2.select(7)==127);

        REQUIRE(rsb2.next_one(2003)==0);
        REQUIRE(rsb2.next_one(180)==199);
        REQUIRE(rsb2.next_one(199)==199);
        REQUIRE(rsb2.next_one(200)==200);


        RSBitmap rsb_static = rsb2.copy();
        rsb_static.make_static();

        REQUIRE(rsb2.rank(1)==rsb_static.rank(1));
        REQUIRE(rsb2.rank(2)==rsb_static.rank(2));
        REQUIRE(rsb2.select(rsb2.rank(1))==rsb_static.select(rsb_static.rank(1)));
        REQUIRE(rsb2.select_n(rsb2.rank_n(1, 2), 2)==rsb_static.select_n(rsb_static.rank_n(1, 2), 2));
        REQUIRE(rsb_static.select(rsb_static.rank(62))==62);
        REQUIRE(rsb_static.select_n(rsb_static.rank_n(100, 2), 2)==100);
        REQUIRE(rsb_static.select(11)==0);
        REQUIRE(rsb_static.select(124)==0);
        REQUIRE(rsb_static.select_n(124, 2)==0);
    }


    SECTION("Fill and check"){
        RSBitmap bit1(500);
        // fill and check
        for(uint64_t c=1;c<bit1.n(); c++){
            CAPTURE(c);
            CAPTURE(bit1.find_slate(c));
            bit1.set(c, true);
            CAPTURE(bit1.ones());
            REQUIRE(bit1.get(c) == true);
            REQUIRE(bit1.get_n(c, 1) == 1);
            REQUIRE(bit1.rank(c) == c-1);
            REQUIRE(bit1.select(bit1.rank(c)) == c);
            // check that no value is overwritten
            if (c%64==1){
                for(uint64_t i=1;i<=c; i++){
                    REQUIRE(bit1.get(i) == true);
                }
            }
        }


    }
}


TEST_CASE( "SegmentedArray") {
    SECTION("unspecialized"){
        std::vector<uint64_t> t({12, 0, 2, 0, 4});
        SegmentedArray<uint32_t, uint8_t, char> segments(t.begin(), t.end(), 123, 0, 't');
        REQUIRE(segments.get_pos(1, 1)==1);
        REQUIRE(segments.get_pos(2, 1)==0);
        REQUIRE(segments.get_pos(3, 1)==13);
        segments.set<0>(segments.get_pos(1, 1), 6);
        segments.set<0>(segments.get_pos(1, 12), 7);
        REQUIRE(segments.get<0>(segments.get_pos(1, 1))==6);
        REQUIRE(segments.get<0>(segments.get_pos(1, 12))==7);
        REQUIRE(segments.get<0>(segments.get_pos(3, 1))==123);
        segments.set<0>(segments.get_pos(3, 1), 6);
        REQUIRE(segments.get<0>(segments.get_pos(3, 1))==6);
        REQUIRE(segments.get<0>(segments.get_pos(1, 12))==7);
        REQUIRE(segments.get<1>(segments.get_pos(1, 1))==0);
        REQUIRE(segments.get<2>(segments.get_pos(1, 1))=='t');
        REQUIRE(segments.segment_size(1)==12);
        REQUIRE(segments.segment_size(2)==0);
        REQUIRE(segments.segment_size(3)==2);
        REQUIRE(segments.segment_size(4)==0);
        REQUIRE(segments.segment_size(5)==4);
    }
    SECTION("boolspecialized"){
        std::vector<uint64_t> t({12, 0, 2, 0, 4});
        SegmentedArray<bool> segments(t.begin(), t.end(), true);
        segments.set<0>(segments.get_pos(1, 1), true);
        segments.set<0>(segments.get_pos(1, 4), true);
        segments.set<0>(segments.get_pos(1, 5), true);
        segments.set<0>(segments.get_pos(1, 8), true);
        segments.set<0>(segments.get_pos(1, 12), true);

        segments.set<0>(segments.get_pos(5, 1), true);
        segments.set<0>(segments.get_pos(5, 4), true);

        REQUIRE(segments.get<0>(segments.get_pos(3, 1))==false);
        segments.set<0>(segments.get_pos(3, 1), true);
        REQUIRE(segments.get<0>(segments.get_pos(3, 1))==true);

        REQUIRE(segments.ones_segment(std::get<0>(segments.arrays), 1)==5);
        REQUIRE(segments.ones_segment(std::get<0>(segments.arrays), 5)==2);



    }
}


TEST_CASE( "Choice Dictionary", "[CD]" ) {
    SECTION("Normal Operations"){
        ChoiceDictionary cd(200);
        // insert
        REQUIRE(cd.insert(0)==false);
        REQUIRE(cd.insert(1)==true);
        REQUIRE(cd.insert(20)==true);
        REQUIRE(cd.insert(100)==true);
        REQUIRE(cd.insert(200)==true);
        // insert errors
        REQUIRE(cd.insert(1)==false);
        REQUIRE(cd.insert(201)==false);
        REQUIRE(cd.insert(250)==false);
        // size and exist
        REQUIRE(cd.size()==4);
        REQUIRE(cd.contains(1)==true);
        // remove
        REQUIRE(cd.remove(1)==true);
        REQUIRE(cd.remove(1)==false);
        REQUIRE(cd.size()==3);
        REQUIRE(cd.contains(1)==false);
        // choice
        REQUIRE(cd.choice()==20);
        REQUIRE(cd.size()==2);
        REQUIRE(cd.insert(40)==true);
        REQUIRE(cd.choice()==40);
        REQUIRE(cd.choice()==100);
        REQUIRE(cd.choice()==200);
        REQUIRE(cd.size()==0);
        // invalid choice
        REQUIRE(cd.choice()==0);
    }
    SECTION("after move"){
        ChoiceDictionary cd(200);
        // insert
        REQUIRE(cd.insert(0)==false);
        REQUIRE(cd.insert(1)==true);
        REQUIRE(cd.insert(20)==true);
        REQUIRE(cd.insert(100)==true);
        REQUIRE(cd.insert(200)==true);
        // insert errors
        REQUIRE(cd.insert(1)==false);
        REQUIRE(cd.insert(201)==false);
        REQUIRE(cd.insert(250)==false);
        // move
        ChoiceDictionary cdmove = std::move(cd);
        // size and exist
        REQUIRE(cdmove.size()==4);
        REQUIRE(cdmove.contains(1)==true);
        // remove
        REQUIRE(cdmove.remove(1)==true);
        REQUIRE(cdmove.remove(1)==false);
        REQUIRE(cdmove.size()==3);
        REQUIRE(cdmove.contains(1)==false);
        // choice
        REQUIRE(cdmove.choice()==20);
        REQUIRE(cdmove.size()==2);
        REQUIRE(cdmove.insert(40)==true);
        REQUIRE(cdmove.choice()==40);
        REQUIRE(cdmove.choice()==100);
        REQUIRE(cdmove.choice()==200);
        REQUIRE(cdmove.size()==0);
        // invalid choice
        REQUIRE(cdmove.choice()==0);
    }
}

TEST_CASE( "SpinStack", "Stack" ) {
    SECTION("Normal Operations"){
        SpinStack<uint64_t> sp(2);
        REQUIRE(sp.empty()==true);
        REQUIRE(sp.size()==0);
        REQUIRE_NOTHROW(sp.push_top(1));
        REQUIRE(sp.empty()==false);
        REQUIRE(sp.full()==false);
        REQUIRE(sp.size()==1);
        REQUIRE_NOTHROW(sp.push_top(2993322));
        REQUIRE(sp.size()==2);
        REQUIRE(sp.empty()==false);
        REQUIRE(sp.full()==true);
        REQUIRE(sp.pop()==2993322);
        REQUIRE(sp.pop()==1);
        REQUIRE(sp.empty()==true);
        REQUIRE(sp.full()==false);
        // test compatibility to move, doesn't compile or crash otherwise
        SpinStack<uint64_t> spmove = std::move(sp);
        REQUIRE(sp.empty()==true);
        REQUIRE(sp.full()==false);
    }
    SECTION("drop front"){
        SpinStack<uint64_t> sp(8);
        REQUIRE(sp.drop_front(10)==0);
        for (uint64_t c=0; c<8; c++){
            CAPTURE(c);
            REQUIRE_NOTHROW(sp.push_top(c*100));
            REQUIRE(sp.size()==c+1);
        }
        REQUIRE(sp.full()==true);
        REQUIRE(sp.size()==8);
        REQUIRE(sp.drop_front(4)==4);
        REQUIRE(sp.size()==4);
        REQUIRE(sp.full()==false);
        for (uint64_t c=8; c<12; c++){
            CAPTURE(c);
            REQUIRE_NOTHROW(sp.push_top(c*100));
        }
        REQUIRE(sp.full()==true);
        for (uint64_t c=11; c>=4; c--)
        {
            CAPTURE(c);
            REQUIRE(sp.pop()==c*100);
            REQUIRE(sp.size()==c-4);
        }
        REQUIRE(sp.empty()==true);

    }
}
