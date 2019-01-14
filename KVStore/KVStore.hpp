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
    Where is the best place to put the logic: in the container (KVStore) or in
the contents (node_t)?
	Is it needed a container at all? Looks like all logic will be inside the node
class anyway.
*/
#include <cstdint>
#include <memory>
template <typename key_type_name, typename value_type_name>
class KVStore {
public:
    static enum uint32_t errors {
        null_key = 1,
        empty_key,
		no_matches,
		not_assigned,
		empty_tree,
    };
    KVStore() : m_root_node(nullptr) {
    }
    ~KVStore();
    uint32_t store_or_update(const key_type_name* key, const size_t key_length, const value_type_name& value) {
        if (!key) return null_key;
        if (!key_length) return empty_key;
        if (!m_root_node) {
			m_root_node = new node_t(key, key_length, value);
			return 0;
        }
		m_root_node->store(key, key_length, value);
		return 0;
    }
    uint32_t retrieve(const key_type_name* key, const size_t key_length, value_type_name& value) {
		if (nullptr == m_root_node) {
			return empty_tree;
		}
		return m_root_node->retrieve(key, key_length, value);
    }
    uint32_t remove(const key_type_name* key, size_t key_length) {
    }
    uint32_t dump() {
    }
    uint32_t balance() {
    }
private:
	class node_t;
	typedef note_t* node_p;
	class node_t {
	public:
		node_t(const key_type_name* key, const size_t key_length, value_type_name& value)
			: m_key_atom(*key)
			, m_assigned(1 < key_length_remaining)
			, m_value(1 < key_length_remaining ? nullptr : new value_type_name(value))
			, m_rightward(nullptr)
			, m_leftward(nullptr)
			, m_downward(1 < key_length ? new node_t(key + 1, key_length - 1, value) : nullptr)
		{
		}
		~node_t() {
			// Is the destruction order meaningful?
			delete m_value;
			delete m_rightward;
			delete m_leftward;
			delete m_downward;
		}
		node_p find_closest(const key_type_name* key, size_t const key_length, size_t& matched_length) const {
			if (*key == m_key_atom) {
				++matched_length;
				if (key_length == matched_length) {
					// Exact match
					return this;
				}
				if (nullptr == m_downward) {
					// Partial match.
					// The tree only has the trailing portion of the key.
					// matched_length contains the length of the trailing portion that matched.
					return this;
				}
				// matched_length elements of key were matched.
				// Go on looking forward the next element of key.
				return m_downward->find_closest(key + 1, key_length, matched_length);
			}
			if (*key > m_key_atom) {
				if (nullptr == m_rightward) {
					// No more places to go.
					// key_length contains the 
					return this;
				}
				return m_rightward->find_closest(key, key_length, matched_length);
			}
			if (nullptr == m_lefttward) {
				// No more places to go.
				return this;
			}
			return m_leftward->find_closest(key, key_length, matched_length);
		}
		uint32_t store(const key_type_name* key, const size_t key_length, const value_type_name& value) {
			size_t matched_length = 0;
			node_p closest_node = find_closest(key, key_length, matched_length);
			if (key_length == matched_length) {
				m_value = value;
				m_assigned = true;
				return 0;
			}
			if (*key == m_key_atom) {
				m_downward = new node_t(key + matched_length, key_length - matched_length, value);
				return 0;
			}
			if (*key > m_key_atom) {
				m_rightward = new node_t(key + matched_length, key_length - matched_length, value);
				return 0;
			}
			m_leftward = new node_t(key + matched_length, key_length - matched_length, value);
			return 0;
		}
		uint32_t retrieve(const key_type_name* key, const size_t key_length, value_type_name& value) {
			size_t matched_length = 0;
			node_p = find_closest(key, key_length, matched_length);
			if (key_length != matched_length) {
				return no_matches;
			}
			if (false == m_assigned) {
				return not_assigned;
			}
			value = m_value;
			return 0;
		}
	private:
        key_type_name m_key_atom;
        bool m_assigned;
        value_type_name* m_value;
        node_p m_rightward;
        node_p m_leftward;
        node_p m_downward;
        // Recursive constructor. How to differentiate when names are shared between properties and parameter?
    };
	// 308 (pagina do livro)
	node_p m_root_node;
};
#endif
