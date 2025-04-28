#include <iostream>
#include <cassert>
#include "bagel.h"
using namespace std;
using namespace bagel;

void test1() {
    ent_type e0 = World::createEntity();
    assert(e0.id == 0 && "First id is not 0");

    ent_type e1 = World::createEntity();
    assert(e1.id == 1 && "Second id is not 1");

    World::destroyEntity(e0);
    e0 = World::createEntity();
    assert(e0.id == 0 && "Id 0 not recycled after destroy & create");

    cout << "Test 1 passed\n";
}

void testDynamicBag() {
    DynamicBag<int, 3> bag;

    const int test_values[] = {10, 20, 30, 40};

    // Test push and size growth
    for (int i = 0; i < 4; ++i) {
        bag.push(test_values[i]);
        assert(bag.size() == i + 1 && "Size should increment with each push");
    }

    // Test element access and capacity
    assert(bag.capacity() >= 4 && "Capacity should be at least 4 after pushing 4 elements");
    for (int i = 0; i < 4; ++i) {
        assert(bag[i] == test_values[i] && "Elements should maintain their order");
    }

    // Test pop
    assert(bag.pop() == 40 && "Last element should be 40");
    assert(bag.pop() == 30 && "Second pop should return 30");
    assert(bag.size() == 2 && "Size should be 2 after two pops");

    // Test clear and ensure
    bag.clear();
    assert(bag.size() == 0 && "Size should be 0 after clear");

    bag.ensure(15);
    assert(bag.capacity() >= 15 && "Capacity should be at least 15 after ensure");

    cout << "DynamicBag tests passed\n";
}

void testPackedStorage() {
    using IntStorage = PackedStorage<int>;

    ent_type entities[3];
    for (int i = 0; i < 3; ++i) {
        entities[i] = World::createEntity();
    }

    // Test component addition and retrieval
    IntStorage::add(entities[0], 100);
    IntStorage::add(entities[1], 200);
    IntStorage::add(entities[2], 300);

    // Verify all components were added correctly
    assert(IntStorage::size() == 3 && "Should have 3 components");
    assert(IntStorage::get(entities[0]) == 100 && "First component should be 100");
    assert(IntStorage::get(entities[1]) == 200 && "Second component should be 200");

    // Test component deletion and reordering
    IntStorage::del(entities[1]);
    assert(IntStorage::size() == 2 && "Size should be 2 after deletion");

    // Verify remaining components
    assert(IntStorage::get(entities[0]) == 100 && "First component should remain unchanged");
    assert(IntStorage::get(entities[2]) == 300 && "Third component should remain unchanged");

    // Test index-based access
    assert(IntStorage::get(0) == 100 && "First index should point to first component");
    assert(IntStorage::entity(1).id == entities[2].id && "Second index should point to third entity");

    cout << "PackedStorage tests passed\n";
}

void run_tests()
{
    test1();
    testDynamicBag();
    testPackedStorage();
}

