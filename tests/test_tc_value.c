#include <math.h>
#include <stdio.h>
#include <string.h>

#include <tcbase/tc_dlist.h>
#include <tcbase/tc_types.h>
#include <tcbase/tc_value.h>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("FAIL: %s (line %d)\n", msg, __LINE__); \
            return 1; \
        } \
    } while (0)

#define EPSILON 1e-9

typedef struct {
    int value;
    tc_dlist_node node;
} ListItem;

static int test_tc_types(void) {
    tc_vec3 v = {1.0, 2.0, 3.0};
    tc_quat q = {0.0, 0.0, 0.0, 1.0};
    tc_pose3 p = {.rotation = q, .position = v};

    TEST_ASSERT(fabs(p.position.x - 1.0) < EPSILON, "tc_pose3.position.x");
    TEST_ASSERT(fabs(p.rotation.w - 1.0) < EPSILON, "tc_pose3.rotation.w");
    return 0;
}

static int test_tc_dlist(void) {
    tc_dlist_head list;
    tc_dlist_init_head(&list);
    TEST_ASSERT(tc_dlist_empty(&list), "list starts empty");

    ListItem a = {.value = 10};
    ListItem b = {.value = 20};
    tc_dlist_init_node(&a.node);
    tc_dlist_init_node(&b.node);

    tc_dlist_add_tail(&a.node, &list);
    tc_dlist_add_tail(&b.node, &list);
    TEST_ASSERT(tc_dlist_size(&list) == 2, "list size after add");
    TEST_ASSERT(tc_dlist_contains(&list, &a.node), "contains first node");

    ListItem* first = tc_dlist_first_entry(&list, ListItem, node);
    ListItem* last = tc_dlist_last_entry(&list, ListItem, node);
    TEST_ASSERT(first != NULL && first->value == 10, "first entry");
    TEST_ASSERT(last != NULL && last->value == 20, "last entry");

    tc_dlist_del(&a.node);
    TEST_ASSERT(tc_dlist_size(&list) == 1, "size after delete");
    TEST_ASSERT(!tc_dlist_contains(&list, &a.node), "deleted node removed");
    return 0;
}

static int test_tc_value(void) {
    tc_value v_nil = tc_value_nil();
    TEST_ASSERT(v_nil.type == TC_VALUE_NIL, "nil type");

    tc_value v_bool = tc_value_bool(true);
    TEST_ASSERT(v_bool.type == TC_VALUE_BOOL, "bool type");
    TEST_ASSERT(v_bool.data.b, "bool value");

    tc_value v_int = tc_value_int(42);
    TEST_ASSERT(v_int.type == TC_VALUE_INT, "int type");
    TEST_ASSERT(v_int.data.i == 42, "int value");

    tc_value v_str = tc_value_string("hello");
    TEST_ASSERT(v_str.type == TC_VALUE_STRING, "string type");
    TEST_ASSERT(strcmp(v_str.data.s, "hello") == 0, "string value");
    tc_value_free(&v_str);

    tc_value v_vec = tc_value_vec3((tc_vec3){1, 2, 3});
    TEST_ASSERT(v_vec.type == TC_VALUE_VEC3, "vec3 type");
    TEST_ASSERT(fabs(v_vec.data.v3.x - 1.0) < EPSILON, "vec3 x");

    tc_value list = tc_value_list_new();
    tc_value_list_push(&list, tc_value_int(1));
    tc_value_list_push(&list, tc_value_int(2));
    tc_value_list_push(&list, tc_value_int(3));
    TEST_ASSERT(tc_value_list_size(&list) == 3, "list count");
    TEST_ASSERT(tc_value_list_get(&list, 1)->data.i == 2, "list get");
    tc_value_free(&list);

    tc_value dict = tc_value_dict_new();
    tc_value_dict_set(&dict, "name", tc_value_string("test"));
    tc_value_dict_set(&dict, "value", tc_value_int(123));
    TEST_ASSERT(tc_value_dict_has(&dict, "name"), "dict has key");
    TEST_ASSERT(tc_value_dict_get(&dict, "value")->data.i == 123, "dict get");
    TEST_ASSERT(tc_value_dict_size(&dict) == 2, "dict size");
    tc_value_free(&dict);

    return 0;
}

int main(void) {
    printf("=== termin-base tests ===\n");

    int result = 0;
    result |= test_tc_types();
    result |= test_tc_dlist();
    result |= test_tc_value();

    if (result == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
    return result;
}
