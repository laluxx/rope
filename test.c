#define ROPE_IMPLEMENTATION
#include "rope.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s...", #name); \
    test_##name(); \
    printf(" OK\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL: %s:%d: %s != %s (%zu != %zu)\n", \
                __FILE__, __LINE__, #a, #b, (size_t)(a), (size_t)(b)); \
        exit(1); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b, len) do { \
    if (memcmp(a, b, len) != 0) { \
        fprintf(stderr, "FAIL: %s:%d: strings differ\n", __FILE__, __LINE__); \
        exit(1); \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s:%d: %s is false\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

/* ============================================================================
 * BASIC OPERATIONS TESTS
 * ========================================================================= */

TEST(create_empty) {
    rope_t *rope = rope_new();
    ASSERT_TRUE(rope != NULL);
    ASSERT_EQ(rope_byte_length(rope), 0);
    ASSERT_EQ(rope_char_length(rope), 0);
    rope_free(rope);
}

TEST(create_from_string) {
    const char *text = "Hello, World!";
    rope_t *rope = rope_new_from_str(text, strlen(text));
    
    ASSERT_EQ(rope_byte_length(rope), 13);
    ASSERT_EQ(rope_char_length(rope), 13);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_EQ(len, 13);
    ASSERT_STR_EQ(result, text, len);
    
    free(result);
    rope_free(rope);
}

TEST(char_at_ascii) {
    rope_t *rope = rope_new_from_str("ABCDEF", 6);
    
    ASSERT_EQ(rope_char_at(rope, 0), 'A');
    ASSERT_EQ(rope_char_at(rope, 2), 'C');
    ASSERT_EQ(rope_char_at(rope, 5), 'F');
    ASSERT_EQ(rope_char_at(rope, 6), 0); /* Out of bounds */
    
    rope_free(rope);
}

TEST(insert_at_start) {
    rope_t *rope = rope_new_from_str("World", 5);
    rope = rope_insert_bytes(rope, 0, "Hello ", 6);
    
    ASSERT_EQ(rope_byte_length(rope), 11);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "Hello World", 11);
    
    free(result);
    rope_free(rope);
}

TEST(insert_at_end) {
    rope_t *rope = rope_new_from_str("Hello", 5);
    rope = rope_insert_bytes(rope, 5, " World", 6);
    
    ASSERT_EQ(rope_byte_length(rope), 11);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "Hello World", 11);
    
    free(result);
    rope_free(rope);
}

TEST(insert_in_middle) {
    rope_t *rope = rope_new_from_str("Helo", 4);
    rope = rope_insert_bytes(rope, 2, "l", 1);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "Hello", 5);
    
    free(result);
    rope_free(rope);
}

TEST(delete_from_start) {
    rope_t *rope = rope_new_from_str("Hello World", 11);
    rope = rope_delete_bytes(rope, 0, 6);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "World", 5);
    
    free(result);
    rope_free(rope);
}

TEST(delete_from_end) {
    rope_t *rope = rope_new_from_str("Hello World", 11);
    rope = rope_delete_bytes(rope, 5, 6);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "Hello", 5);
    
    free(result);
    rope_free(rope);
}

TEST(delete_from_middle) {
    rope_t *rope = rope_new_from_str("Hello World", 11);
    rope = rope_delete_bytes(rope, 5, 1);
    
    ASSERT_EQ(rope_byte_length(rope), 10);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "HelloWorld", 10);
    
    free(result);
    rope_free(rope);
}

TEST(concat_two_ropes) {
    rope_t *left = rope_new_from_str("Hello ", 6);
    rope_t *right = rope_new_from_str("World", 5);
    
    rope_t *result = rope_concat(left, right);
    
    ASSERT_EQ(rope_byte_length(result), 11);
    
    size_t len;
    char *str = rope_to_string(result, &len);
    ASSERT_STR_EQ(str, "Hello World", 11);
    
    free(str);
    rope_free(result);
}

TEST(split_rope) {
    rope_t *rope = rope_new_from_str("Hello World", 11);
    rope_t *right = NULL;
    
    rope_t *left = rope_split_bytes(rope, 6, &right);
    
    ASSERT_EQ(rope_byte_length(left), 6);
    ASSERT_EQ(rope_byte_length(right), 5);
    
    size_t len;
    char *left_str = rope_to_string(left, &len);
    ASSERT_STR_EQ(left_str, "Hello ", 6);
    free(left_str);
    
    char *right_str = rope_to_string(right, &len);
    ASSERT_STR_EQ(right_str, "World", 5);
    free(right_str);
    
    rope_free(left);
    rope_free(right);
}

TEST(zero_length_operations) {
    rope_t *rope = rope_new_from_str("test", 4);
    
    // Zero-length insert
    rope = rope_insert_bytes(rope, 2, "", 0);
    ASSERT_EQ(rope_byte_length(rope), 4);
    
    // Zero-length delete
    rope = rope_delete_bytes(rope, 2, 0);
    ASSERT_EQ(rope_byte_length(rope), 4);
    
    // Copy zero bytes
    char buf[10];
    size_t copied = rope_copy_bytes(rope, 2, 0, buf, 10);
    ASSERT_EQ(copied, 0);
    
    rope_free(rope);
}

/* ============================================================================
 * UTF-8 TESTS
 * ========================================================================= */

TEST(utf8_ascii) {
    rope_t *rope = rope_new_from_str("ASCII", 5);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    ASSERT_EQ(rope_char_length(rope), 5);
    
    rope_free(rope);
}

TEST(utf8_two_byte) {
    /* "café" - é is 2 bytes (0xC3 0xA9) */
    const char *text = "caf\xC3\xA9";
    rope_t *rope = rope_new_from_str(text, 5);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    ASSERT_EQ(rope_char_length(rope), 4);
    
    rope_free(rope);
}

TEST(utf8_three_byte) {
    /* "日本" - each character is 3 bytes */
    const char *text = "\xE6\x97\xA5\xE6\x9C\xAC";
    rope_t *rope = rope_new_from_str(text, 6);
    
    ASSERT_EQ(rope_byte_length(rope), 6);
    ASSERT_EQ(rope_char_length(rope), 2);
    
    rope_free(rope);
}

TEST(utf8_four_byte) {
    /* "𝕳𝖊𝖑𝖑𝖔" - mathematical bold characters, 4 bytes each */
    const char *text = "\xF0\x9D\x95\xB3\xF0\x9D\x96\x8A\xF0\x9D\x96\x91\xF0\x9D\x96\x91\xF0\x9D\x96\x94";
    rope_t *rope = rope_new_from_str(text, 20);
    
    ASSERT_EQ(rope_byte_length(rope), 20);
    ASSERT_EQ(rope_char_length(rope), 5);
    
    rope_free(rope);
}

TEST(utf8_mixed) {
    /* "Hello 世界!" - mix of ASCII, 3-byte UTF-8, and ASCII */
    const char *text = "Hello \xE4\xB8\x96\xE7\x95\x8C!";
    rope_t *rope = rope_new_from_str(text, 13);
    
    ASSERT_EQ(rope_byte_length(rope), 13);
    ASSERT_EQ(rope_char_length(rope), 9); /* H e l l o space 世 界 ! */
    
    rope_free(rope);
}

TEST(utf8_char_to_byte) {
    const char *text = "caf\xC3\xA9"; /* café */
    rope_t *rope = rope_new_from_str(text, 5);
    
    ASSERT_EQ(rope_char_to_byte(rope, 0), 0);
    ASSERT_EQ(rope_char_to_byte(rope, 1), 1);
    ASSERT_EQ(rope_char_to_byte(rope, 2), 2);
    ASSERT_EQ(rope_char_to_byte(rope, 3), 3); /* é starts at byte 3 */
    ASSERT_EQ(rope_char_to_byte(rope, 4), 5); /* After é (2 bytes) */
    
    rope_free(rope);
}

TEST(utf8_byte_to_char) {
    const char *text = "caf\xC3\xA9"; /* café */
    rope_t *rope = rope_new_from_str(text, 5);
    
    ASSERT_EQ(rope_byte_to_char(rope, 0), 0);
    ASSERT_EQ(rope_byte_to_char(rope, 1), 1);
    ASSERT_EQ(rope_byte_to_char(rope, 2), 2);
    ASSERT_EQ(rope_byte_to_char(rope, 3), 3);
    ASSERT_EQ(rope_byte_to_char(rope, 4), 3); /* Mid-character */
    ASSERT_EQ(rope_byte_to_char(rope, 5), 4);
    
    rope_free(rope);
}

TEST(utf8_insert_chars) {
    rope_t *rope = rope_new_from_str("Hello", 5);
    /* Insert "世界" at character position 5 */
    const char *insert = "\xE4\xB8\x96\xE7\x95\x8C";
    rope = rope_insert_chars(rope, 5, insert, 6);
    
    ASSERT_EQ(rope_byte_length(rope), 11);
    ASSERT_EQ(rope_char_length(rope), 7);
    
    rope_free(rope);
}

TEST(utf8_delete_chars) {
    /* "Hello世界" */
    const char *text = "Hello\xE4\xB8\x96\xE7\x95\x8C";
    rope_t *rope = rope_new_from_str(text, 11);
    
    /* Delete "世界" (chars 5-6) */
    rope = rope_delete_chars(rope, 5, 2);
    
    ASSERT_EQ(rope_byte_length(rope), 5);
    ASSERT_EQ(rope_char_length(rope), 5);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "Hello", 5);
    
    free(result);
    rope_free(rope);
}

TEST(utf8_char_at) {
    /* "AB日本" */
    const char *text = "AB\xE6\x97\xA5\xE6\x9C\xAC";
    rope_t *rope = rope_new_from_str(text, 8);
    
    ASSERT_EQ(rope_char_at(rope, 0), 'A');
    ASSERT_EQ(rope_char_at(rope, 1), 'B');
    ASSERT_EQ(rope_char_at(rope, 2), 0x65E5); /* 日 */
    ASSERT_EQ(rope_char_at(rope, 3), 0x672C); /* 本 */
    
    rope_free(rope);
}

TEST(utf8_emojis_combining) {
    /* Test with emojis and combining characters */
    const char *text = "Hello 👨‍👩‍👧‍👦 World! 🎉";
    rope_t *rope = rope_new_from_str(text, strlen(text));
    
    /* Test various operations on complex UTF-8 */
    rope = rope_insert_bytes(rope, 6, "🌍 ", 4);
    rope = rope_delete_bytes(rope, 0, 6);
    
    ASSERT_TRUE(rope_validate_utf8(rope));
    rope_free(rope);
}

TEST(utf8_invalid_recovery) {
    /* Test handling of invalid UTF-8 */
    const char *invalid_utf8 = "Valid\xFF\xFFInvalid";
    rope_t *rope = rope_new_from_str(invalid_utf8, 15);
    
    /* Should handle gracefully even with invalid UTF-8 */
    ASSERT_TRUE(rope_byte_length(rope) == 15);
    
    /* Insert valid UTF-8 after invalid */
    rope = rope_insert_bytes(rope, 10, "Valid", 5);
    
    rope_free(rope);
}

TEST(utf8_overlong_sequences) {
    /* Test handling of overlong UTF-8 sequences */
    const char overlong[] = "\xC0\xAF";  // Overlong '/'
    rope_t *rope = rope_new_from_str(overlong, 2);
    
    // Should handle gracefully without crashing
    ASSERT_EQ(rope_byte_length(rope), 2);
    ASSERT_TRUE(rope_char_length(rope) <= 2);
    
    rope_free(rope);
}

TEST(utf8_incomplete_sequences) {
    /* Test incomplete UTF-8 at end of buffer */
    const char incomplete[] = "test\xE6\x97";  // Incomplete "日"
    rope_t *rope = rope_new_from_str(incomplete, 6);
    
    ASSERT_EQ(rope_byte_length(rope), 6);
    // Should handle the incomplete character gracefully
    
    rope_free(rope);
}


/* ============================================================================
 * LINE OPERATION TESTS
 * ========================================================================= */

TEST(line_count_single) {
    rope_t *rope = rope_new_from_str("Hello", 5);
    ASSERT_EQ(rope_line_count(rope), 1);
    rope_free(rope);
}

TEST(line_count_multiple) {
    rope_t *rope = rope_new_from_str("Line 1\nLine 2\nLine 3", 20);
    ASSERT_EQ(rope_line_count(rope), 3);
    rope_free(rope);
}

TEST(line_count_trailing_newline) {
    rope_t *rope = rope_new_from_str("Line 1\nLine 2\n", 14);
    ASSERT_EQ(rope_line_count(rope), 3); /* Empty line after \n */
    rope_free(rope);
}

TEST(char_to_line) {
    rope_t *rope = rope_new_from_str("Line 1\nLine 2\nLine 3", 20);
    
    ASSERT_EQ(rope_char_to_line(rope, 0), 0);  /* L in Line 1 */
    ASSERT_EQ(rope_char_to_line(rope, 6), 0);  /* \n after Line 1 */
    ASSERT_EQ(rope_char_to_line(rope, 7), 1);  /* L in Line 2 */
    ASSERT_EQ(rope_char_to_line(rope, 14), 2); /* L in Line 3 */
    
    rope_free(rope);
}

TEST(line_to_char) {
    rope_t *rope = rope_new_from_str("Line 1\nLine 2\nLine 3", 20);
    
    ASSERT_EQ(rope_line_to_char(rope, 0), 0);  /* Start of Line 1 */
    ASSERT_EQ(rope_line_to_char(rope, 1), 7);  /* Start of Line 2 */
    ASSERT_EQ(rope_line_to_char(rope, 2), 14); /* Start of Line 3 */
    
    rope_free(rope);
}

/* ============================================================================
 * STRESS TESTS
 * ========================================================================= */

TEST(large_document) {
    rope_t *rope = rope_new();
    
    /* Build a 10KB document */
    const char *line = "This is a test line.\n";
    size_t line_len = strlen(line);
    
    for (int i = 0; i < 500; i++) {
        rope = rope_insert_bytes(rope, rope_byte_length(rope), line, line_len);
    }
    
    ASSERT_EQ(rope_byte_length(rope), line_len * 500);
    ASSERT_EQ(rope_line_count(rope), 501); /* 500 lines + 1 */
    
    rope_free(rope);
}

TEST(many_small_inserts) {
    rope_t *rope = rope_new();
    
    /* Simulate typing */
    const char *text = "The quick brown fox jumps over the lazy dog.";
    size_t len = strlen(text);
    
    for (size_t i = 0; i < len; i++) {
        rope = rope_insert_bytes(rope, i, &text[i], 1);
    }
    
    ASSERT_EQ(rope_byte_length(rope), len);
    
    size_t result_len;
    char *result = rope_to_string(rope, &result_len);
    ASSERT_STR_EQ(result, text, len);
    
    free(result);
    rope_free(rope);
}

TEST(alternating_insert_delete) {
    rope_t *rope = rope_new_from_str("AAAA", 4);
    
    /* Insert and delete alternately */
    for (int i = 0; i < 100; i++) {
        rope = rope_insert_bytes(rope, 2, "BB", 2);
        rope = rope_delete_bytes(rope, 2, 2);
    }
    
    ASSERT_EQ(rope_byte_length(rope), 4);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "AAAA", 4);
    
    free(result);
    rope_free(rope);
}

TEST(split_and_concat_stress) {
    const char *text = "0123456789ABCDEF";
    rope_t *rope = rope_new_from_str(text, 16);
    
    /* Split and rejoin multiple times */
    for (int i = 0; i < 10; i++) {
        rope_t *right = NULL;
        rope = rope_split_bytes(rope, 8, &right);
        rope = rope_concat(rope, right);
    }
    
    ASSERT_EQ(rope_byte_length(rope), 16);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, text, 16);
    
    free(result);
    rope_free(rope);
}

TEST(utf8_stress) {
    rope_t *rope = rope_new();
    
    /* Build document with mixed UTF-8 */
    const char *parts[] = {
        "ASCII text ",
        "café ",
        "日本語 ",
        "🚀 ",
        "مرحبا "
    };
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            size_t len = strlen(parts[j]);
            rope = rope_insert_bytes(rope, rope_byte_length(rope), parts[j], len);
        }
    }
    
    /* Verify we can still access everything */
    ASSERT_TRUE(rope_byte_length(rope) > 0);
    ASSERT_TRUE(rope_char_length(rope) > 0);
    ASSERT_TRUE(rope_char_length(rope) < rope_byte_length(rope));
    
    rope_free(rope);
}

/* ============================================================================
 * EDGE CASE TESTS
 * ========================================================================= */

TEST(empty_operations) {
    rope_t *rope = rope_new();
    
    ASSERT_EQ(rope_byte_length(rope), 0);
    ASSERT_EQ(rope_char_length(rope), 0);
    ASSERT_EQ(rope_char_at(rope, 0), 0);
    ASSERT_EQ(rope_line_count(rope), 1);
    
    /* Insert into empty */
    rope = rope_insert_bytes(rope, 0, "X", 1);
    ASSERT_EQ(rope_byte_length(rope), 1);
    
    /* Delete everything */
    rope = rope_delete_bytes(rope, 0, 1);
    ASSERT_EQ(rope_byte_length(rope), 0);
    
    rope_free(rope);
}

TEST(boundary_inserts) {
    rope_t *rope = rope_new_from_str("ABC", 3);
    
    /* Insert at every position */
    rope = rope_insert_bytes(rope, 0, "0", 1);  /* 0ABC */
    rope = rope_insert_bytes(rope, 4, "4", 1);  /* 0ABC4 */
    rope = rope_insert_bytes(rope, 2, "2", 1);  /* 0A2BC4 */
    
    ASSERT_EQ(rope_byte_length(rope), 6);
    
    size_t len;
    char *result = rope_to_string(rope, &len);
    ASSERT_STR_EQ(result, "0A2BC4", 6);
    
    free(result);
    rope_free(rope);
}

TEST(single_character) {
    rope_t *rope = rope_new_from_str("X", 1);
    
    ASSERT_EQ(rope_byte_length(rope), 1);
    ASSERT_EQ(rope_char_at(rope, 0), 'X');
    
    rope = rope_delete_bytes(rope, 0, 1);
    ASSERT_EQ(rope_byte_length(rope), 0);
    
    rope_free(rope);
}

TEST(copy_operations) {
    const char *text = "Hello World";
    rope_t *rope = rope_new_from_str(text, 11);
    
    char buf[100];
    size_t copied;
    
    /* Copy full string */
    copied = rope_copy_bytes(rope, 0, 11, buf, 100);
    ASSERT_EQ(copied, 11);
    ASSERT_STR_EQ(buf, text, 11);
    
    /* Copy substring */
    copied = rope_copy_bytes(rope, 6, 5, buf, 100);
    ASSERT_EQ(copied, 5);
    ASSERT_STR_EQ(buf, "World", 5);
    
    /* Copy with small buffer */
    copied = rope_copy_bytes(rope, 0, 11, buf, 5);
    ASSERT_EQ(copied, 5);
    ASSERT_STR_EQ(buf, "Hello", 5);
    
    rope_free(rope);
}

/* ============================================================================
 * ITERATOR TESTS
 * ========================================================================= */

TEST(iterator_forward) {
    const char *text = "ABC";
    rope_t *rope = rope_new_from_str(text, 3);
    
    rope_iter_t iter;
    rope_iter_init(&iter, rope, 0);
    
    uint32_t c;
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 'A');
    
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 'B');
    
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 'C');
    
    ASSERT_TRUE(!rope_iter_next_char(&iter, &c)); /* End */
    
    if (iter.internal) free(iter.internal);
    rope_free(rope);
}

TEST(iterator_utf8) {
    /* "A日B" */
    const char *text = "A\xE6\x97\xA5" "B";
    rope_t *rope = rope_new_from_str(text, 5);
    
    rope_iter_t iter;
    rope_iter_init(&iter, rope, 0);
    
    uint32_t c;
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 'A');
    
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 0x65E5); /* 日 */
    
    ASSERT_TRUE(rope_iter_next_char(&iter, &c));
    ASSERT_EQ(c, 'B');
    
    if (iter.internal) free(iter.internal);
    rope_free(rope);
}


TEST(iterator_state_persistence) {
    rope_t *rope = rope_new_from_str("Hello World", 11);
    rope_iter_t iter;
    
    // Test that iterator maintains state correctly
    rope_iter_init(&iter, rope, 0);
    
    uint32_t c;
    rope_iter_next_char(&iter, &c); // H
    rope_iter_next_char(&iter, &c); // e
    
    // Seek should reset state properly
    rope_iter_seek_char(&iter, 6);
    rope_iter_next_char(&iter, &c);
    ASSERT_EQ(c, 'W');
    
    if (iter.internal) free(iter.internal);
    rope_free(rope);
}

TEST(iterator_concurrent) {
    /* Multiple iterators on same rope */
    rope_t *rope = rope_new_from_str("ABCDEF", 6);
    
    rope_iter_t iter1, iter2;
    rope_iter_init(&iter1, rope, 0);
    rope_iter_init(&iter2, rope, 3);
    
    uint32_t c1, c2;
    rope_iter_next_char(&iter1, &c1); // A
    rope_iter_next_char(&iter2, &c2); // D
    
    ASSERT_EQ(c1, 'A');
    ASSERT_EQ(c2, 'D');
    
    if (iter1.internal) free(iter1.internal);
    if (iter2.internal) free(iter2.internal);
    rope_free(rope);
}

TEST(iterator_edge_cases) {
    rope_t *rope = rope_new_from_str("ABC", 3);
    rope_iter_t iter;
    
    /* Test iterator at end */
    rope_iter_init(&iter, rope, 3);
    uint32_t c;
    ASSERT_TRUE(!rope_iter_next_char(&iter, &c));
    
    /* Test iterator at beginning with prev */
    rope_iter_init(&iter, rope, 0);
    ASSERT_TRUE(!rope_iter_prev_char(&iter, &c));
    
    if (iter.internal) free(iter.internal);
    rope_free(rope);
}

/* ============================================================================
 * COMPLEX OPERATIONS TESTS
 * ========================================================================= */

TEST(multiple_splits_and_merges) {
    rope_t *rope = rope_new_from_str("0123456789", 10);
    
    rope_t *part1 = NULL, *part2 = NULL, *part3 = NULL;
    
    // Split into: "012", "3456789"
    rope_t *left = rope_split_bytes(rope, 3, &part1);
    
    // Split "3456789" into: "345", "6789"  
    rope_t *mid = rope_split_bytes(part1, 3, &part2);
    
    // Split "6789" into: "67", "89"
    rope_t *right = rope_split_bytes(part2, 2, &part3);
    
    /* Reassemble as: "345" + "012" + "67" + "89" = "3450126789" */
    rope_t *reassembled = rope_concat(mid, left);
    reassembled = rope_concat(reassembled, right);  
    reassembled = rope_concat(reassembled, part3);
    
    size_t len;
    char *result = rope_to_string(reassembled, &len);
    ASSERT_STR_EQ(result, "3450126789", 10);
    
    free(result);
    rope_free(reassembled);
}

/* ============================================================================
 * PERFORMANCE TESTS
 * ========================================================================= */

TEST(performance_large_inserts) {
    rope_t *rope = rope_new();
    
    /* Insert 1MB of data */
    const char *chunk = "This is a test chunk. ";
    size_t chunk_len = strlen(chunk);
    
    for (int i = 0; i < 50000; i++) {  // ~1.1MB
        rope = rope_insert_bytes(rope, rope_byte_length(rope), chunk, chunk_len);
    }
    
    ASSERT_TRUE(rope_byte_length(rope) > 1000000);
    rope_free(rope);
}

TEST(performance_random_access) {
    /* Build large rope and test random character access */
    rope_t *rope = rope_new();
    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
    
    for (int i = 0; i < 10000; i++) {
        rope = rope_insert_bytes(rope, rope_byte_length(rope), alphabet, 26);
    }
    
    /* Access random positions */
    for (int i = 0; i < 1000; i++) {
        size_t pos = rand() % rope_char_length(rope);
        uint32_t c = rope_char_at(rope, pos);
        ASSERT_TRUE(c >= 'a' && c <= 'z');
    }
    
    rope_free(rope);
}

/* ============================================================================
 * NO CATEGORY
 * ========================================================================= */

TEST(tree_invariants_after_operations) {
    /* Test that the tree remains balanced after various operations */
    rope_t *rope = rope_new();
    
    // Insert in reverse order (worst case for balancing)
    for (int i = 9; i >= 0; i--) {
        char c = '0' + i;
        rope = rope_insert_bytes(rope, 0, &c, 1);
    }
    
    // Delete from middle
    rope = rope_delete_bytes(rope, 3, 4);
    
    // Verify we can still access all characters
    for (int i = 0; i < rope_char_length(rope); i++) {
        uint32_t c = rope_char_at(rope, i);
        ASSERT_TRUE(c >= '0' && c <= '9');
    }
    
    rope_free(rope);
}

TEST(node_pool_reuse) {
    /* Test that node pool is properly used */
    rope_t *ropes[10];
    
    // Create and destroy multiple ropes
    for (int i = 0; i < 10; i++) {
        ropes[i] = rope_new_from_str("test string", 11);
    }
    
    // Free them (should go into pool)
    for (int i = 0; i < 10; i++) {
        rope_free(ropes[i]);
    }
    
    // Create new ropes (should reuse from pool)
    for (int i = 0; i < 5; i++) {
        ropes[i] = rope_new_from_str("reused", 6);
        ASSERT_TRUE(ropes[i] != NULL);
    }
    
    for (int i = 0; i < 5; i++) {
        rope_free(ropes[i]);
    }
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ========================================================================= */

int main(void) {
    printf("Running rope.h test suite...\n\n");

    // Basic operations
    RUN_TEST(create_empty);
    RUN_TEST(create_from_string);
    RUN_TEST(char_at_ascii);
    RUN_TEST(insert_at_start);
    RUN_TEST(insert_at_end);
    RUN_TEST(insert_in_middle);
    RUN_TEST(delete_from_start);
    RUN_TEST(delete_from_end);
    RUN_TEST(delete_from_middle);
    RUN_TEST(concat_two_ropes);
    RUN_TEST(split_rope);
    RUN_TEST(zero_length_operations);
    
    // UTF-8 tests
    RUN_TEST(utf8_ascii);
    RUN_TEST(utf8_two_byte);
    RUN_TEST(utf8_three_byte);
    RUN_TEST(utf8_four_byte);
    RUN_TEST(utf8_mixed);
    RUN_TEST(utf8_char_to_byte);
    RUN_TEST(utf8_byte_to_char);
    RUN_TEST(utf8_insert_chars);
    RUN_TEST(utf8_delete_chars);
    RUN_TEST(utf8_char_at);
    RUN_TEST(utf8_overlong_sequences);
    RUN_TEST(utf8_incomplete_sequences);
    
    // Advanced UTF-8 tests
    RUN_TEST(utf8_emojis_combining);
    RUN_TEST(utf8_invalid_recovery);
    
    // Line operations
    RUN_TEST(line_count_single);
    RUN_TEST(line_count_multiple);
    RUN_TEST(line_count_trailing_newline);
    RUN_TEST(char_to_line);
    RUN_TEST(line_to_char);
    
    // Stress tests
    RUN_TEST(large_document);
    RUN_TEST(many_small_inserts);
    RUN_TEST(alternating_insert_delete);
    RUN_TEST(split_and_concat_stress);
    RUN_TEST(utf8_stress);
    
    // Performance tests
    RUN_TEST(performance_large_inserts);
    RUN_TEST(performance_random_access);
    
    // Complex operations
    RUN_TEST(multiple_splits_and_merges);
    
    // Edge cases
    RUN_TEST(empty_operations);
    RUN_TEST(boundary_inserts);
    RUN_TEST(single_character);
    RUN_TEST(copy_operations);
    
    // Iterator tests
    RUN_TEST(iterator_forward);
    RUN_TEST(iterator_utf8);
    RUN_TEST(iterator_state_persistence);
    RUN_TEST(iterator_concurrent);
    RUN_TEST(iterator_edge_cases);

    RUN_TEST(node_pool_reuse);
    RUN_TEST(tree_invariants_after_operations);
    
    printf("\n✓ All tests passed!\n");
    return 0;
}
