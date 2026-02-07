#include <gtest/gtest.h>
#include <shared_resources/shared_resources.hpp>

using all      = srs::type_list<int, char, int *, char *>;
using shuffled = srs::type_list<char *, int, char, int *>;
using extra    = srs::type_list<int, char *, char **, int *, char>;

TEST(shared_resources_test, get)
{
    int a   = 1;
    char b  = 'a';
    int *c  = new int(2);
    char *d = new char('b');
    srs::shared_resources<all> resources(c, b, d, a);
    EXPECT_EQ(resources.get<int>(), 1);
    EXPECT_EQ(resources.get<char>(), 'a');
    EXPECT_EQ(*resources.get<int *>(), 2);
    EXPECT_EQ(*resources.get<char *>(), 'b');
    *resources.get<int *>() = 3;
    EXPECT_EQ(*c, 3);
    resources.get<char>() = 'c';
    EXPECT_EQ(resources.get<char>(), 'c');

    delete c;
    delete d;
}

TEST(shared_resources_test, convert)
{
    int a   = 1;
    char b  = 'a';
    int *c  = new int(2);
    char *d = new char('b');
    srs::shared_resources<all, int> int_resources(c, b, d);
    srs::shared_resources<all> resources(int_resources, a);
    EXPECT_EQ(resources.get<int>(), 1);
    srs::shared_resources<all, char *> charp_resources(int_resources, a);
    EXPECT_EQ(charp_resources.get<int>(), 1);
    EXPECT_EQ(charp_resources.get<char>(), 'a');

    srs::shared_resources<shuffled> shuffled_resources(int_resources, a);
    EXPECT_EQ(shuffled_resources.get<int>(), 1);
    EXPECT_EQ(shuffled_resources.get<char>(), 'a');

    srs::shared_resources<extra, char **, int> extra_int_resources(int_resources);
    EXPECT_EQ(extra_int_resources.get<char>(), 'a');
    srs::shared_resources<extra, char **> extra_resources(int_resources, a);
    EXPECT_EQ(extra_resources.get<int>(), 1);

    delete c;
    delete d;
}

TEST(shared_resources_test, references)
{
    int a   = 1;
    char b  = 'a';
    int *c  = new int(2);
    char *d = new char('b');

    srs::shared_references<all, int> int_references(c, b, d);
    EXPECT_EQ(int_references.get<char>(), 'a');
    EXPECT_EQ(*int_references.get<int *>(), 2);
    EXPECT_EQ(*int_references.get<char *>(), 'b');

    srs::shared_references<all> references(int_references, a);
    EXPECT_EQ(references.get<int>(), 1);

    references.get<char>() = 'c';
    EXPECT_EQ(b, 'c');

    auto copy(references);
    EXPECT_EQ(copy.get<int>(), 1);
    EXPECT_EQ(copy.get<char>(), 'c');

    delete c;
    delete d;
}
