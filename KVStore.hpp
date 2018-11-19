#ifndef KVStore_hpp
#define KVStore_hpp
/*
    The objective is to create a simple key value store.
    To improve search speed and maybe storage efficiency the organization will
be as follows:
    Suppose there are the following keys of type int8_t which represent the
following list of words:
    "Quick setup - if you’ve done this kind of thing before"
    Inserting one by one will generate the successive trees:

               Q           
              /|\          
             / u s         
            /  | |\        
           /   i e \       
          /    | |  \      
         /     c t   \     
        /      | u    \    
       /       k |     \   
      /          |      y  
     /           p     /|  
    /                 / o  
   -                 /  |  
    \               t   u  
     \              |   |  
      \             h   '  
       \            |   |  
        \           i   v  
         i          |\  |  
        /|\         s n e  
       d f k          |    
      /|   |\         g    
     b O   i o             
     | |   | |             
     e n   n f             
     | |   |               
     f e   d               
     |                     
     o                     
     |                     
     r                     
     |                     
     e                     
                           

    In the above graph, slashes and backslashes denote traveling in the same
level (all first letters of every word for instance) and pipes mean going from
one level to the next (from the first letter to the second letter for instance).
    For a little group of words there is no gain at all (much probably ther will
be even loss). But with a large enough set of words, the reuse of nodes like
what can be seen with "this" and "thing" will become prevalent.
    The great gain however happens when the tree is balanced and a search is
done because in this case, for instance, if all 256 symbols are used for the
int8_t type then a word with 10 symbols will take up to 80 compares to be found,
worst case scenario, regardless of the number of words in the tree.

    v_* are the internal state properties.
    f_* are the internal methods.
    *_t are the internal types.

    Is it valuable make node_t a simple data structure or a full fledged class?
    Is it valuable differentiate store from update or merge both in one single
method?
    Is it valuable to wrap the multiple pointers to node_t as std::unique_ptr to
guarantee that they will be automatically freed when no more used? What is the
performance penalty?
    Is it possible to restrict the valid types for key_type_name to only signed
and unsigned integer types wit 8, 16, 32 and 64 bit?
*/
#include <cstdint>
#include <memory>
template <typename key_type_name, typename value_type_name>
class KVStore {
public:
    static enum uint32_t errors {
        null_key = 1,
        empty_key,
    };
    KVStore() : v_root_node(nullptr) {
    }
    ~KVStore();
    uint32_t store_or_update(const key_type_name* key, size_t key_length, const value_type_name& value) {
        if (!key) return null_key;
        if (!key_length) return empty_key;
        if (!v_root_node) {
            return f_store(key, key_length, value);
        }
        size_t closest_key_length = 0;
        note_t* closest_node = f_find_closest_node(key, key_length, closest_key_length);
        return f_store(closest_node, key, key_length, value);
    }
    uint32_t retrieve(const key_type_name* key, size_t key_length, value_type_name& value) {
    }
    uint32_t remove(const key_type_name* key, size_t key_length) {
    }
    uint32_t dump() {
    }
    uint32_t balance() {
    }
private:
    class node_t {
        key_type_name key_atom;
        bool assigned;
        value_type_name* value;
        node_t* rightward;
        node_t* leftward;
        node_t* downward;
        // Recursive constructor. How to differentiate when names are shared between properties and parameter?
        node_t(const key_type_name* key, size_t key_length_remaining, value_type_name& value)
            : key_atom(*key)
            , assigned(1 < key_length_remaining)
            , value(1 < key_length_remaining ? nullptr : new value_type_name(value))
            , rightward(nullptr)
            , leftward(nullptr)
            , downward(1 < key_length_remaining ? new node_t(key + 1, key_length_remaining - 1, value) : nullptr)
        {
        }
        ~node_t() {
            // Is the destruction order meaningful?
            delete value;
            delete rightward;
            delete leftward;
            delete downward;
        }
    };
    node_t_p v_root_node;
    uint32_t f_store(const key_type_name* key, size_t key_length, const value_type_name& value) {
        
    }
    uint32_t f_store(node_t* closest_node, const key_type_name* key, size_t key_length, const value_type_name& value) {
    }
    v_root_node& f_find_closest_node(const key_type_name* key, size_t key_length, size_t& closest_key_length) {
    }
};
#endif
