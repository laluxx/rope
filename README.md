# rope.h - UTF-8 Rope Data Structure

A single-header rope implementation for text editors, featuring proper UTF-8 handling and Red-Black tree balancing.

## Why Ropes?

Traditional string buffers become painfully slow for large documents. Inserting text at position N in a flat buffer requires moving O(N) bytes. Ropes solve this by representing text as a binary tree where leaves contain string fragments. This gives O(log n) insertion, deletion, and indexing operations.

## Architecture

### Node Structure

Each node is either a leaf (containing actual text) or a branch (pointing to two children). Branch nodes cache several metrics about their left subtree:

- `byte_weight` - total bytes in left subtree
- `char_weight` - total UTF-8 characters in left subtree  
- `newline_weight` - total newlines in left subtree

These cached weights make navigation fast. To find byte position 5000, we don't need to traverse every leaf - we can skip entire subtrees by comparing against their weight.

### Red-Black Balancing

We use Red-Black trees rather than AVL because they require fewer rotations during modifications. Text editors perform many sequential inserts (typing), and RB-trees handle this well with at most 2 rotations per insertion.

The balancing maintains these invariants:
1. Root is always black
2. Red nodes cannot have red children
3. All paths from root to null have the same black height

When inserting, we allow temporary violations of rule 2, then fix them with rotations and color flips. The `balance()` function handles three cases:
- Right-leaning red (rotate left)
- Two reds on left path (rotate right)  
- Both children red (flip colors)

### UTF-8 Handling

UTF-8 is variable-width, so character position ≠ byte position. We maintain both metrics throughout the tree. When converting between them, we walk the tree using cached weights to narrow down to a leaf, then scan that leaf.

Character decoding respects multi-byte sequences:
- `0xxxxxxx` - 1 byte (ASCII)
- `110xxxxx 10xxxxxx` - 2 bytes
- `1110xxxx 10xxxxxx 10xxxxxx` - 3 bytes
- `11110xxx 10xxxxxx 10xxxxxx 10xxxxxx` - 4 bytes

The decoder validates continuation bytes (`10xxxxxx` pattern) and returns U+FFFD for invalid sequences.

### Split Algorithm

Splitting is the trickiest operation. Given a rope and position P, we need two new ropes representing [0,P) and [P,end).

The recursive algorithm:
1. If splitting a leaf, create two new leaves from the byte fragments
2. If P is in the left subtree, split that subtree and combine results with the right subtree
3. If P is in the right subtree, split that and combine with the left subtree

Key insight: we recursively decompose the problem until hitting leaves, then reassemble on the way back up. The implementation reuses nodes where possible to avoid allocations.

### Node Pooling

Text editors create and destroy many nodes during editing. We maintain a pool of freed nodes (up to 512) and reuse them instead of calling malloc/free repeatedly. This reduces allocator pressure significantly.

## API Design

The library exposes both byte-level and character-level operations:

```c
// Byte operations - fast, direct
rope_insert_bytes(rope, byte_pos, str, len);
rope_delete_bytes(rope, start, len);

// Character operations - UTF-8 aware
rope_insert_chars(rope, char_pos, str, len);  
rope_delete_chars(rope, start, len);
```

Character operations convert positions internally but maintain the abstraction that you're working with Unicode codepoints, not bytes.

## Performance Characteristics

| Operation  | Complexity | Notes                         |
|------------|------------|-------------------------------|
| Insert     | O(log n)   | Plus balancing overhead       |
| Delete     | O(log n)   | Implemented as split + concat |
| Index      | O(log n)   | Following cached weights      |
| Concat     | O(1)       | Just create a branch node     |
| Split      | O(log n)   | Recursive decomposition       |
| Length     | O(1)       | Cached at root                |
| Line count | O(1)       | Cached newlines               |

For a 1MB document (~1M chars), log₂(1M) ≈ 20, so worst-case is ~20 node visits.

## What We Don't Do

### No Copy-on-Write

Real production ropes often implement COW semantics so undo/redo can share structure between document versions. We always create new nodes. Adding COW would require reference counting and careful handling of modification.

### Simplified Iterator

The iterator doesn't maintain a proper traversal stack - it recalculates position on each access. A real iterator would cache the path through the tree and update it incrementally. Current approach works but isn't optimal for sequential access.

### No Rebalancing Heuristics

We balance on every insert. Some implementations only rebalance when imbalance exceeds a threshold, or batch rebalancing during idle time. This would reduce overhead for bulk operations.

### No Gap Buffer Leaves

Hybrid approaches use gap buffers (or piece tables) within leaves for better cache locality during sequential editing. Pure rope leaves require allocating new strings on modification.

### Line Operations Aren't Cached

Line-to-byte conversions currently scan the rope. We could cache line start positions in a separate index structure for O(log n) line access instead of O(n).

### No Grapheme Cluster Awareness

We work with Unicode codepoints, not grapheme clusters. "é" might be one codepoint (U+00E9) or two (e + combining acute). Real text editors need grapheme-aware cursor movement.

### Memory Fragmentation

Leaves are individually allocated. After many edits, memory becomes fragmented. Arena allocators or slab allocation would help, but complicate the single-header design.

## Integration Notes

### Undo/Redo

The current API consumes ropes on modification (nulls the root after concat/split). To implement undo, I'd need to either:
1. Convert to COW with reference counting
2. Keep copies of the entire rope (memory-heavy)
3. Store inverse operations (insert ↔ delete) as commands

### Rendering

For syntax highlighting, you need to iterate chunks of text while maintaining a parsing state. The iterator API supports this, but you'll want to cache token boundaries.

### Multi-Cursor Editing

When applying multiple cursors' edits, positions shift as earlier edits occur. Either:
- Apply edits back-to-front (stable positions)
- Track position mappings through a transformation system (Operational Transform style)

### Persistent Storage

Ropes are memory structures. For saving to disk, either:
- Convert to flat string (`rope_to_string`)
- Serialize tree structure if you need to preserve exact node boundaries (rare)

## Configuration

Tune these macros before including:

```c
#define ROPE_NODE_SIZE 1024           // Leaf capacity
#define ROPE_SPLIT_THRESHOLD 2048     // When to split large inserts
#define NODE_POOL_SIZE 512            // Max pooled nodes
```

`ROPE_NODE_SIZE` affects cache behavior. Smaller = more nodes = worse locality. Larger = fewer nodes = more wasted space in partially-filled leaves. 1KB works well for typical L1 cache sizes.

## Testing Recommendations

Key test cases:
- Insert/delete at boundaries (0, length, middle)
- UTF-8 sequences (ASCII, 2-byte, 3-byte, 4-byte, invalid)
- Splits that land mid-character (should never happen with correct APIs)
- Large documents (>1M chars) to verify O(log n) behavior
- Pathological editing patterns (many small inserts at document start)
- Line operations near beginning, middle, end

## Known Issues

1. **Iterator state lifetime** - Iterator holds raw pointer to rope. If rope is freed while iterator exists, undefined behavior. Consider making iterators hold a reference or document lifetime rules clearly.

2. **No thread safety** - Concurrent access requires external synchronization. Multiple readers would be safe (tree is immutable during reads), but no reader/writer synchronization exists.

3. **Leaf splitting in insert** - When inserting mid-leaf, we create three nodes (left, new, right) then branch them. This adds an extra level. Could optimize by growing the leaf in-place if capacity allows.

4. **Traversal in to_string** - Uses explicit stack which limits depth to 64. Deeply unbalanced trees (shouldn't happen with RB) could overflow. Could use dynamic stack or Morris traversal.

## Future Enhancements

**Worth doing:**
- Proper iterator with cached stack
- COW semantics for undo/redo
- Piece table integration for better sequential edit performance
- Line start index for fast line-based access

**Maybe worth doing:**
- Grapheme cluster support (complex, Unicode tables required)
- Concurrent read access with RCU-style updates
- SIMD-accelerated UTF-8 validation and scanning
- mmap'd persistent rope for huge files
